#ifndef INCLUDE_ROCO2_TASK_EXPERIMENT_TASK_HPP
#define INCLUDE_ROCO2_TASK_EXPERIMENT_TASK_HPP

#include <roco2/task/task.hpp>

#include <roco2/experiments/base.hpp>

namespace roco2
{
namespace task
{
    class experiment_task : public task
    {
    public:
        experiment_task(roco2::experiments::base& exp, roco2::kernels::base_kernel& kernel,
                        const roco2::experiments::cpu_sets::cpu_set& on)
        : exp_(exp), kernel_(kernel), on_(on)
        {
        }

        virtual roco2::chrono::duration eta() const override
        {
            return exp_.eta();
        }

        virtual void execute() override
        {
            exp_.run(kernel_, on_);
        }

        auto tag() -> auto
        {
            return kernel_.tag();
        }

    private:
        roco2::experiments::base& exp_;
        roco2::kernels::base_kernel& kernel_;
        roco2::experiments::cpu_sets::cpu_set on_;
    };
} // namespace task
} // namespace roco2

#endif // INCLUDE_ROCO2_TASK_EXPERIMENT_TASK_HPP
