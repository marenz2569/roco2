#ifndef INCLUDE_ROCO2_MEMORY_HPP
#define INCLUDE_ROCO2_MEMORY_HPP

#include <firestarter/LoadWorkerMemory.hpp>
#include <roco2/memory/alignment_allocator.hpp>

#include <omp.h>

#include <memory>
#include <vector>

namespace roco2
{

namespace detail
{

    struct thread_local_memory
    {
        thread_local_memory()
        : vec_A(vec_size), vec_B(vec_size), vec_C(vec_size), vec_F(vec_size),
          mat_A(mat_size * mat_size), mat_B(mat_size * mat_size), mat_C(mat_size * mat_size),
          mem_buffer(mem_size)
        {
            for (std::size_t i = 0; i < vec_A.size(); ++i)
            {
                vec_A[i] = static_cast<double>(i) * 0.3;
                vec_B[i] = static_cast<double>(i) * 0.2;
                vec_C[i] = static_cast<double>(i) * 0.7;
                vec_F[i] = static_cast<float>(i) * 1.42f;
            }

            for (std::size_t i = 0; i < mat_A.size(); i++)
            {
                mat_A[i] = static_cast<double>(i + 1);
                mat_B[i] = static_cast<double>(i + 1);
                mat_C[i] = static_cast<double>(i + 1);
            }

            for (std::size_t i = 0; i < mem_buffer.size(); i++)
            {
                mem_buffer[i] = i * 23 + 42;
            }

            log::debug() << "Memory allocated and touched.";
        }

        std::vector<double> vec_A;
        std::vector<double> vec_B;
        std::vector<double> vec_C;
        std::vector<float> vec_F;

        std::vector<double> mat_A;
        std::vector<double> mat_B;
        std::vector<double> mat_C;

        std::vector<std::uint64_t> mem_buffer;

        ::firestarter::LoadWorkerMemory::UniquePtr firestarter_memory = { nullptr, nullptr };

        const static std::size_t vec_size = 1024;
        const static std::size_t mat_size = 512;

        // size of mem_buffer equals 64MB
        const static std::size_t mem_size = 64 * 1024 * 1024 / sizeof(mem_buffer[0]);
    };
} // namespace detail

static detail::thread_local_memory& thread_local_memory()
{
    static std::vector<std::unique_ptr<detail::thread_local_memory>> memory(omp_get_max_threads());

    auto& tld = memory[omp_get_thread_num()];

    if (!tld)
    {
        tld.reset(new detail::thread_local_memory());
    }

    return *tld;
}
} // namespace roco2
#endif // INCLUDE_ROCO2_MEMORY_HPP
