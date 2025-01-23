#include <firestarter/Measurement/Summary.hpp>

#include "roco2/metrics/meta.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace roco2
{
namespace metrics
{
    /// Thiis class describes one line in the csv
    struct storage_entry
    {
        storage_entry()
        {
            metric_values.emplace_back(std::to_string(meta::instance().experiment));
            metric_values.emplace_back(std::to_string(meta::instance().frequency));
            metric_values.emplace_back(std::to_string(meta::instance().shell));
            metric_values.emplace_back(std::to_string(meta::instance().threads));
            metric_values.emplace_back(std::to_string(meta::instance().utility));
            metric_values.emplace_back(std::to_string(meta::instance().op1));
            metric_values.emplace_back(std::to_string(meta::instance().op2));
        }

        std::vector<std::string> metric_values;
    };

    /// The storage class that handles saving the metrics of roco2 and the firestarter measurement
    /// worker for external metrics.
    class storage
    {
    private:
        storage() = default;

        /// The list of metrics we save, prepopulated with the metrics of roco2.
        std::vector<std::string> metric_names = { "experiment", "frequency", "shell", "threads",
                                                  "utility",    "op1",       "op2" };

        std::vector<storage_entry> metric_entries;

    public:
        storage(const storage&) = delete;
        storage& operator=(const storage&) = delete;

        static storage& instance()
        {
            // gcc doesn't like omp threadprivate classes
            static thread_local storage instance;
            return instance;
        }

        /// Register additional metrics in this storage
        void add_metrics(const std::vector<std::string>& additional_metric_names)
        {
            for (const auto& name : additional_metric_names)
            {
                metric_names.emplace_back(name);
            }
        }

        /// Save the metrics of firestarter and the internal metrics of roco2
        void
        save(const std::map<std::string, ::firestarter::measurement::Summary>& firestarter_metrics)
        {
            storage_entry entry;

            for (const auto& metric_name : metric_names)
            {
                auto is_in_firestarter_metrics = firestarter_metrics.count(metric_name);
                if (is_in_firestarter_metrics)
                {
                    entry.metric_values.emplace_back(
                        std::to_string(firestarter_metrics.at(metric_name).Average));
                }
            }

            metric_entries.emplace_back(entry);
        }

        void print_last()
        {
            std::stringstream ss;
            for (const auto& name : metric_names)
            {
                ss << name << ',';
            }
            ss << '\n';

            const auto back_iterator = metric_entries.back();

            for (const auto& value : back_iterator.metric_values)
            {
                ss << value << ',';
            }

            log::info() << ss.str();
        }
    };
} // namespace metrics
} // namespace roco2