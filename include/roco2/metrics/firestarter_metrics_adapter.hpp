#ifndef INCLUDE_ROCO2_METRICS_FIRESTARTER_METRICS_ADAPTER_HPP
#define INCLUDE_ROCO2_METRICS_FIRESTARTER_METRICS_ADAPTER_HPP

#include <firestarter/Measurement/MeasurementWorker.hpp>
#include <firestarter/Measurement/Summary.hpp>

#include "roco2/metrics/storage.hpp"

#include <string>
#include <vector>

namespace roco2
{
namespace metrics
{
    /// This class provides an adapter to the internal firestarter metric collection.
    class firestarter_metrics_adapter
    {
    private:
        firestarter_metrics_adapter(
            std::chrono::milliseconds update_interval = std::chrono::milliseconds(10),
            std::chrono::milliseconds start_delta = std::chrono::milliseconds(100),
            std::chrono::milliseconds stop_delta = std::chrono::milliseconds(100),
            const std::vector<std::string>& metric_dylib_names = std::vector<std::string>(),
            const std::vector<std::string>& stdin_metric_names = std::vector<std::string>())
        : start_delta(start_delta), stop_delta(stop_delta)
        {
            // TODO: make these variables configurable
            measurement_worker = std::make_unique<::firestarter::measurement::MeasurementWorker>(
                /*UpdateInterval=*/update_interval, /*NumThreads=*/1,
                /*MetricDylibsNames=*/metric_dylib_names,
                /*StdinMetricsNames=*/stdin_metric_names);

            const auto metrics = measurement_worker->metricNames();
            roco2::metrics::storage::instance().add_metrics(metrics);

            measurement_worker->initMetrics(metrics);
        }

        std::unique_ptr<::firestarter::measurement::MeasurementWorker> measurement_worker;
        std::chrono::milliseconds start_delta;
        std::chrono::milliseconds stop_delta;

    public:
        firestarter_metrics_adapter(const firestarter_metrics_adapter&) = delete;
        firestarter_metrics_adapter& operator=(const firestarter_metrics_adapter&) = delete;

        static firestarter_metrics_adapter& instance()
        {
            static firestarter_metrics_adapter instance;
            return instance;
        }

        void start_measurement()
        {
#pragma omp barrier
#pragma omp master
            {
                measurement_worker->startMeasurement();
            }
        }

        void finish_measurement()
        {
#pragma omp barrier
#pragma omp master
            {
                const auto summary = measurement_worker->getValues(start_delta, stop_delta);

                roco2::metrics::storage::instance().save(summary);
                roco2::metrics::storage::instance().print_last();
            }
        }
    };
} // namespace metrics
} // namespace roco2

#endif