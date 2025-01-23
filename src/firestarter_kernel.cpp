#include <roco2/kernels/firestarter.hpp>
#include <roco2/memory/thread_local.hpp>
#include <roco2/metrics/utility.hpp>

#include <roco2/scorep.hpp>

#include <memory>

#include <firestarter/CPUTopology.hpp>
#include <firestarter/Constants.hpp>
#include <firestarter/ProcessorInformation.hpp>
#include <firestarter/X86/X86FunctionSelection.hpp>
#include <firestarter/X86/X86ProcessorInformation.hpp>

namespace roco2
{
namespace kernels
{

    firestarter::firestarter()
    {
        ::firestarter::CPUTopology topology;
        std::unique_ptr<::firestarter::ProcessorInformation> processor_infos =
            std::make_unique<::firestarter::x86::X86ProcessorInformation>();

        auto function_ptr =
            ::firestarter::x86::X86FunctionSelection().selectDefaultOrFallbackFunction(
                processor_infos->cpuModel(), processor_infos->cpuFeatures(),
                processor_infos->vendor(), processor_infos->model(),
                topology.instructionCacheSize(),
                topology.homogenousResourceCount().NumThreadsPerCore);

        const auto& function_const_ref = function_ptr->constRef();

        compiled_payload_ptr = function_const_ref.payload()->compilePayload(
            function_const_ref.settings(), /*DumpRegisters=*/false,
            /*ErrorDetection=*/false, /*PrintAssembler=*/false);

        auto buffersize_mem = function_const_ref.settings().totalBufferSizePerThread();

        auto& memory = roco2::thread_local_memory().firestarter_memory;
        memory = ::firestarter::LoadWorkerMemory::allocate(buffersize_mem);

        compiled_payload_ptr->init(memory->getMemoryAddress(), buffersize_mem / sizeof(uint64_t));
    }

    void firestarter::stop_kernel(roco2::chrono::time_point until,
                                  ::firestarter::LoadThreadWorkType& load_var)
    {
        std::this_thread::sleep_for(until - std::chrono::high_resolution_clock::now());
        load_var = ::firestarter::LoadThreadWorkType::LoadStop;
    }

    void firestarter::run_kernel(roco2::chrono::time_point until)
    {
#ifdef HAS_SCOREP
        SCOREP_USER_REGION("firestarter_kernel", SCOREP_USER_REGION_TYPE_FUNCTION)
#endif
        load_var = ::firestarter::LoadThreadWorkType::LoadHigh;

        // Create a new thread that terminates the load worker after the experiment time has
        // elapsed
        auto cntrl_thread = std::thread(&firestarter::stop_kernel, until, std::ref(load_var));

        const auto& memory = roco2::thread_local_memory().firestarter_memory;

        auto iterations =
            compiled_payload_ptr->highLoadFunction(memory->getMemoryAddress(), load_var,
                                                   /*Iterations=*/0);

        roco2::metrics::utility::instance().write(iterations);

        // Wait for the termination thread to join.
        cntrl_thread.join();
    }
} // namespace kernels
} // namespace roco2
