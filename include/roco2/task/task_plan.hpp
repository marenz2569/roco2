#ifndef INCLUDE_ROCO2_TASK_TASK_PLAN_HPP
#define INCLUDE_ROCO2_TASK_TASK_PLAN_HPP

#include <firestarter/Measurement/MeasurementWorker.hpp>
#include <roco2/chrono/chrono.hpp>
#include <roco2/log.hpp>
#include <roco2/task/experiment_task.hpp>
#include <roco2/task/task.hpp>

#include <memory>
#include <vector>

namespace roco2
{
namespace task
{
    class task_plan
    {
    public:
        task_plan()
        {
#pragma omp master
            {
                // TODO: make these variables configurable
                measurement_worker =
                    std::make_unique<::firestarter::measurement::MeasurementWorker>(
                        /*UpdateInterval=*/std::chrono::milliseconds(10), omp_get_num_threads(),
                        /*MetricDylibsNames=*/std::vector<std::string>(),
                        /*StdinMetricsNames=*/std::vector<std::string>());

                const auto metrics = measurement_worker->metricNames();
                measurement_worker->initMetrics(metrics);
            }
        }

        roco2::chrono::duration eta() const
        {
            return eta_;
        }

        void push_back(std::unique_ptr<task>&& t)
        {
            eta_ += t->eta();
            tasks_.push_back(std::move(t));
        }

        template <typename Task>
        void push_back(Task&& t)
        {
            push_back(static_cast<std::unique_ptr<task>>(std::make_unique<Task>(t)));
        }

        void execute()
        {
            for (auto& task : tasks_)
            {
#pragma omp barrier
#pragma omp master
                {
                    log::info() << "ETA: "
                                << std::chrono::duration_cast<std::chrono::seconds>(eta_);

                    if (auto* exp_task = dynamic_cast<roco2::task::experiment_task*>(task.get()))
                    {
                        log::info() << "Task tag: " << exp_task->tag();
                    }

                    measurement_worker->startMeasurement();
                }

                task->execute();
                eta_ -= task->eta();

#pragma omp barrier
#pragma omp master
                {
                    // TODO: make these variables configurable
                    const auto summary = measurement_worker->getValues(
                        std::chrono::milliseconds::zero(), std::chrono::milliseconds::zero());

                    log::info() << "metric,num_timepoints,duration_ms,average,stddev";
                    for (auto const& [name, sum] : summary)
                    {
                        log::info()
                            << std::quoted(name) << "," << sum.NumTimepoints << ","
                            << sum.Duration.count() << "," << sum.Average << "," << sum.Stddev;
                    }
                }
            }

            executed_ = true;
        }

    private:
        std::unique_ptr<::firestarter::measurement::MeasurementWorker> measurement_worker;
        bool executed_ = false;
        std::vector<std::unique_ptr<task>> tasks_;
        roco2::chrono::duration eta_ = roco2::chrono::duration(0);
    };
} // namespace task
} // namespace roco2

#endif // INCLUDE_ROCO2_TASK_TASK_HPP
