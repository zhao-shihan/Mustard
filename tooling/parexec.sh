#!/usr/bin/env bash

parexec() {
    local use_hwthreads=false
    local filtered_args=()
    for arg in "$@"; do
        case $arg in
            --use-hwthreads)
                use_hwthreads=true
                ;;
            *)
                filtered_args+=("$arg")
                ;;
        esac
    done
    if $use_hwthreads; then
        # Use hardware threads (hyperthreading included)
        if mpiexec --version 2>/dev/null | grep -q "Open MPI"; then
            mpiexec --allow-run-as-root --use-hwthread-cpus "${filtered_args[@]}"
        else
            mpiexec -n "$(nproc)" "${filtered_args[@]}"
        fi
    else
        # Use physical cores (default)
        local threads_per_core=$(env LC_ALL=C lscpu | grep "Thread(s) per core" | awk '{print $4}')
        if [[ -z "$threads_per_core" || "$threads_per_core" -eq 0 ]]; then
            threads_per_core=1  # Fallback to assuming 1 thread per core
        fi
        local n_physical_cores=$(( $(nproc) / threads_per_core ))
        if [[ "$n_physical_cores" -lt 1 ]]; then
            n_physical_cores=1  # Ensure at least 1 core
        fi
        if mpiexec --version 2>/dev/null | grep -q "Open MPI"; then
            mpiexec --allow-run-as-root -n "$n_physical_cores" "${filtered_args[@]}"
        else
            mpiexec -n "$n_physical_cores" "${filtered_args[@]}"
        fi
    fi
}
