#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <stdint.h>

// -------------------------------------------------------------------
uint64_t ackermann(const uint64_t m, const uint64_t n) {
    if (m == 0)   return n+1;
    if (n == 0)   return ackermann(m-1, 1);
    return ackermann(m - 1, ackermann(m, n-1));
}

// -------------------------------------------------------------------
static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
		int cpu, int group_fd, unsigned long flags)
{
  int ret;

  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
		group_fd, flags);
  return ret;
}

int open_event(__u32 event_type, __u64 event)
{
  struct perf_event_attr pe;
  long long count;
  int fd;

  memset(&pe, 0, sizeof(pe));
  pe.type = event_type;
  pe.size = sizeof(pe);
  pe.config = event;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;

  fd = perf_event_open(&pe, 0, -1, -1, 0);

  if (fd == -1) {
    fprintf(stderr, "Error opening leader %llx\n", pe.config);
    exit(EXIT_FAILURE);
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

  return fd;
}

void reset_event(int fd)
{
  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
}

long long read_event(int fd)
{
  long long count;
  read(fd, &count, sizeof(count));
  return count;
}

void close_event(int fd)
{
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
  close(fd);
}

int
main(int argc, char *argv[])
{
  int fd_cycles = open_event(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
  int fd_instr  = open_event(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);

  long long cycles, instr;

  printf("ackermann=%llu\n", ackermann(4, 1));
  cycles = read_event(fd_cycles);
  instr  = read_event(fd_instr);
  printf("Used %lld cycles\n", cycles);
  printf("Used %lld instructions\n", instr);

  printf("ackermann=%llu\n", ackermann(4, 1));
  cycles = read_event(fd_cycles);
  instr  = read_event(fd_instr);
  printf("Used %lld cycles\n", cycles);
  printf("Used %lld instructions\n", instr);

  reset_event(fd_cycles);
  reset_event(fd_instr);
  
  printf("ackermann=%llu\n", ackermann(4, 1));
  cycles = read_event(fd_cycles);
  instr  = read_event(fd_instr);
  printf("Used %lld cycles\n", cycles);
  printf("Used %lld instructions\n", instr);

  printf("ackermann=%llu\n", ackermann(4, 1));
  cycles = read_event(fd_cycles);
  instr  = read_event(fd_instr);
  printf("Used %lld cycles\n", cycles);
  printf("Used %lld instructions\n", instr);

  close_event(fd_cycles);
  close_event(fd_instr);
}
