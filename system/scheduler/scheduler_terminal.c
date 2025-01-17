#include "scheduler_internal.h"

#if SCH_CFG_ENABLE_TERMINAL
#include "term_table.h"

void Sch_AddCmdToCli(EmbeddedCli *cli) {
#if SCH_CFG_ENABLE_TASK
  static CliCommandBinding sch_cmd = {
      .name = "task",
      .usage =
          "task [-l list | -e enable | -d disable | -r delete | -f "
          "setfreq | -p setpri | -E excute] "
          "[taskname] [freq|pri]",
      .help = "Task control command (Scheduler)",
      .context = NULL,
      .autoTokenizeArgs = 1,
      .func = task_cmd_func,
  };

  embeddedCliAddBinding(cli, sch_cmd);
#endif  // SCH_CFG_ENABLE_TASK

#if SCH_CFG_ENABLE_EVENT
  static CliCommandBinding event_cmd = {
      .name = "event",
      .usage =
          "event [-l list | -e enable | -d disable | -r delete | -t trigger] "
          "[eventname] [type] "
          "[content]",
      .help = "Event control command (Scheduler)",
      .context = NULL,
      .autoTokenizeArgs = 1,
      .func = event_cmd_func,
  };

  embeddedCliAddBinding(cli, event_cmd);
#endif  // SCH_CFG_ENABLE_EVENT

#if SCH_CFG_ENABLE_COROUTINE
  static CliCommandBinding cortn_cmd = {
      .name = "cortn",
      .usage = "cortn [-l list| -s stop] [name]",
      .help = "Coroutine control command (Scheduler)",
      .context = NULL,
      .autoTokenizeArgs = 1,
      .func = cortn_cmd_func,
  };

  embeddedCliAddBinding(cli, cortn_cmd);
#endif  // SCH_CFG_ENABLE_COROUTINE

#if SCH_CFG_ENABLE_SOFTINT
  static CliCommandBinding softint_cmd = {
      .name = "softint",
      .usage = "softint [channel] [sub-channel]",
      .help = "SoftInt manual trigger command (Scheduler)",
      .context = NULL,
      .autoTokenizeArgs = 1,
      .func = softint_cmd_func,
  };

  embeddedCliAddBinding(cli, softint_cmd);
#endif  // SCH_CFG_ENABLE_SOFTINT
}
#endif  // SCH_CFG_ENABLE_TERMINAL
