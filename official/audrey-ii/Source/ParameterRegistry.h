#pragma once
#ifndef INFS_CONTROLS_H
#define INFS_CONTROLS_H

#include <functional>
#include <unordered_map>
#include <daisysp.h>
#include "SmoothedValue.h"
#include "DSPUtils.h"

namespace infrasonic {

/**
 * @brief
 * Generic control parameter registry with built in update smoothing
 *
 * @tparam ParamId Template parameter specifying the parameter ID type
 */
template<typename ParamId>
class ParameterRegistry {

    public:

        using Handler = std::function<void(const float)>;

        ParameterRegistry() {};
        ~ParameterRegistry() {};

        void Init(const float control_rate)
        {
            control_rate_ = control_rate;
        }

        void Register(const ParamId id, const float initial_value, const float min, const float max,
                      Handler handler, const float smooth_time = 0.05f,
                      const daisysp::Mapping mapping = daisysp::Mapping::LINEAR)
        {
            const float coef = onepole_coef_t60(smooth_time, control_rate_);
            ParamState state(initial_value, min, max, coef, mapping, handler);
            param_states_.insert({id, state});
        }

        // Called in response to new control input. Updates target, does not apply value.
        void Update(const ParamId id, const float value, const bool immediate = false)
        {
            auto it = param_states_.find(id);
            if (it != param_states_.end()) {
                auto &state = it->second;
                auto clampedValue = daisysp::fclamp(value, state.min, state.max);
                state.value.set(clampedValue, immediate);
            }
        }

        void UpdateNormalized(const ParamId id, const float normValue, const bool immediate = false)
        {
            auto it = param_states_.find(id);
            if (it != param_states_.end()) {
                auto &state = it->second;
                auto mappedValue = daisysp::fmap(daisysp::fclamp(normValue, 0.0f, 1.0f), state.min, state.max, state.mapping);
                state.value.set(mappedValue, immediate);
            }
        }

        // Updates param values with smoothing and applies using handler.
        void Process()
        {
            for (auto &param : param_states_) {
                auto &state = param.second;
                state.handler(state.value.get());
            }
        }

    private:

        static constexpr float kParamSmoothingThresh = 0.001f;

        struct ParamState {
            SmoothedValue value;

            const float min;
            const float max;
            const daisysp::Mapping mapping;
            const Handler handler;

            ParamState() = delete;
            ParamState(const float initial, const float min, const float max,
                       const float coef, const daisysp::Mapping mapping, const Handler handler)
                : value(initial, coef)
                , min(min)
                , max(max)
                , mapping(mapping)
                , handler(handler)
            {
            }
        };

        // TODO: redo this to use libDaisy Parameter objects and
        // avoid using STL container
        using ParamStates = std::unordered_map<ParamId, ParamState>;

        float control_rate_;
        ParamStates param_states_;
};

}

#endif
