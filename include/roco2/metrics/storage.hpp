#ifndef INCLUDE_ROCO2_METRICS_STORAGE_HPP
#define INCLUDE_ROCO2_METRICS_STORAGE_HPP

#include <firestarter/Measurement/Summary.hpp>

#include "roco2/metrics/meta.hpp"

#include <fstream>
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
            static storage instance;
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
                    auto metric_value = firestarter_metrics.at(metric_name).Average;
                    if (metric_name == "perf-freq")
                    {
                        // firestarter metric collection does not cope with dynamic number of
                        // threads yet.
                        entry.metric_values.emplace_back(
                            std::to_string(metric_value / meta::instance().threads));
                    }
                    else
                    {
                        entry.metric_values.emplace_back(std::to_string(metric_value));
                    }
                }
            }

            metric_entries.emplace_back(entry);
        }

        /// Print the csv with one line containing only the latest metrics
        void print_last() const
        {
            std::stringstream ss;
            for (const auto& name : metric_names)
            {
                ss << name << ',';
            }
            ss.seekp(-1, std::ios_base::end);
            ss << '\n';

            const auto back_iterator = metric_entries.back();

            for (const auto& value : back_iterator.metric_values)
            {
                ss << value << ',';
            }
            ss.seekp(-1, std::ios_base::end);
            ss << ' ';

            log::info() << ss.str();
        }

        /// Save all the stored metrics to a csv file.
        /// \arg outpath The path to which the csv is saved.
        void save_csv(const std::string& outpath) const
        {
            std::stringstream ss;
            for (const auto& name : metric_names)
            {
                ss << name << ',';
            }
            ss.seekp(-1, std::ios_base::end);
            ss << '\n';

            for (const auto& entry : metric_entries)
            {
                for (const auto& value : entry.metric_values)
                {
                    ss << value << ',';
                }
                ss.seekp(-1, std::ios_base::end);
                ss << '\n';
            }

            ss.seekp(-1, std::ios_base::end);
            ss << ' ';

            log::info() << "Saving measurement data to " << outpath;

            std::ofstream fp(outpath);

            if (fp.bad())
            {
                log::error() << "Could not open " << outpath;
                return;
            }

            fp << ss.str();

            fp.close();
        }
    };
} // namespace metrics
} // namespace roco2

#endif