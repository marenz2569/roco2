#ifndef INCLUDE_ROCO2_TASK_TASK_PLAN_HPP
#define INCLUDE_ROCO2_TASK_TASK_PLAN_HPP

#include <firestarter/Measurement/MeasurementWorker.hpp>
#include <roco2/chrono/chrono.hpp>
#include <roco2/log.hpp>
#include <roco2/metrics/storage.hpp>
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
        task_plan(std::chrono::milliseconds update_interval = std::chrono::milliseconds(10),
                  const std::vector<std::string>& metric_dylib_names = std::vector<std::string>(),
                  const std::vector<std::string>& stdin_metric_names = std::vector<std::string>())
        {
#pragma omp master
            {
                // TODO: make these variables configurable
                measurement_worker =
                    std::make_unique<::firestarter::measurement::MeasurementWorker>(
                        /*UpdateInterval=*/update_interval, omp_get_num_threads(),
                        /*MetricDylibsNames=*/metric_dylib_names,
                        /*StdinMetricsNames=*/stdin_metric_names);

                const auto metrics = measurement_worker->metricNames();
                roco2::metrics::storage::instance().add_metrics(metrics);

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

                    roco2::metrics::storage::instance().save(summary);

                    roco2::metrics::storage::instance().print_last();
                }
            }

            executed_ = true;
        }

        void save_csv(const std::string& outpath) const
        {
            roco2::metrics::storage::instance().save_csv(outpath);
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
