# SysCore: Mini Operating System with Integrated Data Management

SysCore is a college project that combines operating system concepts with database management using C, Assembly, and Oracle SQL.

## Project Objectives

- Develop a basic bootable operating system kernel.
- Implement a command-line interface for interacting with the kernel.
- Demonstrate basic task management.
- Implement CPU scheduling algorithms.
- Store process information and system logs in an Oracle database.

## Technologies Used

- C
- x86 Assembly
- GCC Cross-Compiler (i686-elf-gcc)
- QEMU
- GRUB / Multiboot
- Oracle Database 11g XE
- SQL*Plus
- Ubuntu on WSL

## Current Features

### Operating System
- Bootable kernel running in QEMU.
- Basic text-based shell.
- Commands for help, system information, and screen clearing.
- Basic task creation and listing.
- FCFS, SJF, Priority, and Round Robin scheduling demonstrations.

### Database Management
- USERS table.
- PROCESSES table.
- PROCESS_HISTORY table.
- SYSTEM_LOG table.
- Sample records and SQL queries.
- Relationships between users and processes using foreign keys.

# SysCore: Mini Operating System with Integrated Data Management

SysCore is a college project that combines operating system concepts with database management using C, Assembly, and Oracle SQL.

## Project Objectives

- Develop a basic bootable operating system kernel.
- Implement a command-line interface for interacting with the kernel.
- Demonstrate basic task management.
- Implement CPU scheduling algorithms.
- Store process information and system logs in an Oracle database.

## Technologies Used

- C
- x86 Assembly
- GCC Cross-Compiler (i686-elf-gcc)
- QEMU
- GRUB / Multiboot
- Oracle Database 11g XE
- SQL*Plus
- Ubuntu on WSL

## Current Features

### Operating System
- Bootable kernel running in QEMU.
- Basic text-based shell.
- Commands for help, system information, and screen clearing.
- Basic task creation and listing.
- FCFS, SJF, Priority, and Round Robin scheduling demonstrations.

### Database Management
- USERS table.
- PROCESSES table.
- PROCESS_HISTORY table.
- SYSTEM_LOG table.
- Sample records and SQL queries.
- Relationships between users and processes using foreign keys.

## Current Limitations

- Scheduling algorithms currently demonstrate scheduling calculations rather than actual CPU context switching.
- Timer interrupt integration is still under development.
- Direct integration between the operating system and Oracle database is not yet complete.

## Project Status

Under development as part of a B.Tech Computer Science Engineering project.

## Disclaimer

This is an educational project intended to demonstrate fundamental operating system and database management concepts.
