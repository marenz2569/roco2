#ifndef INCLUDE_ROCO2_KERNELS_FIRESTARTER_HPP
#define INCLUDE_ROCO2_KERNELS_FIRESTARTER_HPP

#include <roco2/chrono/util.hpp>
#include <roco2/kernels/base_kernel.hpp>

#include <firestarter/Payload/CompiledPayload.hpp>

namespace roco2
{
namespace kernels
{

    class firestarter : public base_kernel
    {

        using param_type = unsigned long long;

    public:
        firestarter();

        virtual experiment_tag tag() const override
        {
            return 6;
        }

    private:
        /// Runs the firestarter kernel until a timepoint
        /// \arg until The timepoint until the kernel should execute the high load function of
        /// firestarter
        void run_kernel(roco2::chrono::time_point until) override;

        /// This function terminates the execution of firestarter by writing the load variable to
        /// ::firestarter::LoadThreadWorkType::LoadStop after the time of the experiment elapsed.
        /// \arg until The timepoint until the kernel should execute the high load function of
        /// firestarter
        /// \arg load_var the reference to the variable that termination the load of firestarter
        static void stop_kernel(roco2::chrono::time_point until,
                                ::firestarter::LoadThreadWorkType& load_var);

        /// The variable shared across all threads that termination the load of firestarter
        ::firestarter::LoadThreadWorkType load_var = ::firestarter::LoadThreadWorkType::LoadHigh;

        /// The unique ptr to the load and init function for firestarter
        ::firestarter::payload::CompiledPayload::UniquePtr compiled_payload_ptr = { nullptr,
                                                                                    nullptr };
    };
} // namespace kernels
} // namespace roco2

#endif // INCLUDE_ROCO2_KERNELS_FIRESTARTER_HPP
