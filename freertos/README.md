# Custom RTOS Implementations

Custom Real-Time Operating Systems built from scratch in C and ARM assembly for STM32 microcontrollers.

## What This Is

This repository contains bare-metal RTOS implementations that I built to understand operating system internals. Each RTOS includes a kernel, schedulers, context switching, and synchronization primitives—all written without using existing RTOS frameworks.

## What's Implemented

**Schedulers:**
- Round-robin (time-sliced)
- Cooperative (voluntary yielding)
- Periodic (time-triggered)
- Priority-based (preemptive)

**Kernel Features:**
- Thread control blocks and management
- Context switching (ARM assembly)
- Thread synchronization (semaphores, mailboxes, queues)
- Hardware abstraction (GPIO, UART, timers, ADC)

**Hardware:**
- STM32F429ZI Nucleo board
- STM32F411 Discovery board

## How It Works

The RTOS manages multiple threads by:
1. Maintaining a thread control block (TCB) for each thread with its stack pointer and state
2. Using SysTick timer interrupts to trigger context switches
3. Saving/restoring CPU registers in assembly to switch between threads
4. Implementing various scheduling policies to determine which thread runs next

Context switching happens in the PendSV exception handler, which saves the current thread's registers to its stack, updates the TCB pointer, and restores the next thread's registers.