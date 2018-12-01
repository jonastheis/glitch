typedef struct
{
  GLuint *counterList;
  int numCounters;
  int maxActiveCounters;
} CounterInfo;

void counters_init();
void init_perf_functions();
void dump_counter_names(int verbose);
void init_groups_counters(GLuint **groupsList, int *numGroups, CounterInfo **counterInfo);

void perform_measurement(GLuint *target_groups, GLuint *target_counters, int num_target_counters);
void measure_counters(GLuint monitor, GLuint *target_groups, GLuint *target_counters, GLuint num_target_counters, int verbose); 
