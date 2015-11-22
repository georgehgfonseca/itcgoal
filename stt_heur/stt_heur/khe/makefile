############################################################################
#                                                                          #
#  Make file for compiling KHE                                             #
#                                                                          #
#  Jeffrey H. Kingston                                                     #
#                                                                          #
#     make              Compile KHE                                        #
#     make khe          Ditto                                              #
#     make clean        Undo "make khe".                                   #
#                                                                          #
#  Although "make khe" produces binary "khe", which invokes the main()     #
#  function in khe_main.c, you can throw away that binary, throw away      #
#  file khe_main.o, and link the other .o files to your own code.  You     #
#  lose nothing by doing that except main().                               #
#                                                                          #
#  KHE uses the pthread system library.  If you don't have that library,   #
#  you can still compile KHE, by doing this:                               #
#                                                                          #
#  (1) Remove "-pthread" from the CFLAGS line just below.                  #
#  (2) Remove "-pthread" from another point further down.                  #
#  (3) Find line "#define KHE_USE_PTHREAD 1" near the top of file khe.h    #
#      and change the 1 to a 0.                                            #
#                                                                          #
#  The only difference will be that function KheParallelSolve() will       #
#  solve its multiple solutions sequentially instead of in parallel.       #
#                                                                          #
#  Mail jeff@it.usyd.edu.au if you have any problems.                      #
#                                                                          #
############################################################################

CC = gcc

CFLAGS = -std=c99 -pedantic -Wall -pthread -O3


############################################################################
#                                                                          #
#   Separate files                                                         #
#                                                                          #
#   Separate files are source files with their own header files that       #
#   could be extracted from KHE and used separately:                       #
#                                                                          #
#     File               Contents                                 Guide    #
#     -----------------------------------------------------------------    #
#     m.[hc]              Extensible arrays and symbol tables     App A    #
#     khe_lset.[hc]       Extensible bit vectors                  App B    #
#     khe_priqueue.[hc]   Updateable priority queues              App C    #
#     kml.[hc]            XML reading and writing                 App D    #
#     khe_wmatch.[hc]     Weighted bipartite matching                      #
#     khe_partition.[hc]  Partitions (e.g. 2 1 is partition of 3)          #
#     -----------------------------------------------------------------    #
#                                                                          #
############################################################################

SEPARATE_OBJS = m.o kml.o khe_lset.o khe_wmatch.o khe_partition.o	\
  khe_priqueue.o


############################################################################
#                                                                          #
#  Platform files                                                          #
#                                                                          #
#  Platform files implement KHE's platform.  They include file             #
#  khe_interns.h, which includes khe.h and the header files of all the     #
#  separate modules except khe_partition.h and khe_priqueue.h.             #
#                                                                          #
#     Files              Contents                                 Guide    #
#     -----------------------------------------------------------------    #
#     INSTANCE_OBJS      Archives, soln groups, instances        Ch 2-3    #
#     CONSTRAINT_OBJS    Constraints                             Sc 3.5    #
#     SOLN_OBJS          Solns                                   Ch 4-5    #
#     MONITOR_OBJS       Monitors                                Ch 6-8    #
#     -----------------------------------------------------------------    #
#                                                                          #
############################################################################

ORDINARY_HDRS = khe_interns.h khe.h kml.h m.h khe_lset.h khe_matching.h    \
  khe_wmatch.h

INSTANCE_OBJS = khe_archive.o khe_archive_metadata.o khe_soln_group.o	   \
  khe_soln_group_metadata.o khe_instance.o khe_instance_metadata.o	   \
  khe_time_group_nhood.o khe_time_group.o khe_time.o khe_resource_type.o   \
  khe_resource_group.o khe_resource.o khe_event_group.o khe_event.o	   \
  khe_event_resource.o khe_event_resource_group.o

CONSTRAINT_OBJS = khe_constraint.o					   \
  khe_assign_resource_constraint.o khe_assign_time_constraint.o		   \
  khe_split_events_constraint.o khe_distribute_split_events_constraint.o   \
  khe_prefer_resources_constraint.o khe_prefer_times_constraint.o	   \
  khe_avoid_split_assignments_constraint.o khe_spread_events_constraint.o  \
  khe_link_events_constraint.o khe_avoid_clashes_constraint.o		   \
  khe_avoid_unavailable_times_constraint.o				   \
  khe_limit_idle_times_constraint.o khe_cluster_busy_times_constraint.o	   \
  khe_limit_busy_times_constraint.o khe_limit_workload_constraint.o

SOLN_OBJS = khe_soln.o khe_evenness_handler.o khe_meet.o khe_cycle_meet.o  \
  khe_task.o khe_transaction.o khe_trace.o khe_node.o khe_layer.o	   \
  khe_zone.o khe_tasking.o khe_event_in_soln.o				   \
  khe_event_resource_in_soln.o khe_resource_in_soln.o khe_matching.o

MONITOR_OBJS = khe_monitor.o khe_dev_monitor.o				   \
  khe_assign_resource_monitor.o khe_assign_time_monitor.o		   \
  khe_split_events_monitor.o khe_distribute_split_events_monitor.o	   \
  khe_prefer_resources_monitor.o khe_prefer_times_monitor.o		   \
  khe_avoid_split_assignments_monitor.o khe_spread_events_monitor.o	   \
  khe_link_events_monitor.o khe_avoid_clashes_monitor.o			   \
  khe_avoid_unavailable_times_monitor.o khe_limit_idle_times_monitor.o	   \
  khe_cluster_busy_times_monitor.o khe_limit_busy_times_monitor.o	   \
  khe_limit_workload_monitor.o khe_timetable_monitor.o			   \
  khe_time_group_monitor.o khe_group_monitor.o				   \
  khe_ordinary_demand_monitor.o khe_workload_demand_monitor.o		   \
  khe_evenness_monitor.o

ORDINARY_OBJS = $(INSTANCE_OBJS) $(CONSTRAINT_OBJS) $(SOLN_OBJS) $(MONITOR_OBJS)


############################################################################
#                                                                          #
#   Layer tree construction files                                          #
#                                                                          #
#   Layer tree construction files contain the implementation of the        #
#   layer tree construction algorithm (Sect. 9.1).  That algorithm is      #
#   a solver, so these files, like all solver files, depend on khe.h       #
#   but not on khe_interns.h.  They have their own header file,            #
#   khe_layer_tree.h, which includes khe.h, m.h, and khe_partition.h.      #
#                                                                          #
############################################################################

LAYER_TREE_HDRS = khe_layer_tree.h khe.h m.h khe_partition.h

LAYER_TREE_OBJS = khe_layer_tree.o khe_split_forest.o khe_split_class.o	   \
  khe_split_layer.o khe_split_task.o 					   \
  khe_avoid_clashes_split_task.o khe_pack_split_task.o			   \
  khe_preassigned_split_task.o khe_assigned_split_task.o		   \
  khe_link_split_task.o khe_split_split_task.o khe_distribute_split_task.o \
  khe_spread_split_task.o khe_domain_split_task.o


############################################################################
#                                                                          #
#   Other solver files                                                     #
#                                                                          #
#   As explained in the User's Guide, solvers should not depend on the     #
#   internals of KHE:  they should include khe.h but not khe_interns.h.    #
#   The solvers packaged with KHE obey this rule.  They also use m.h.      #
#                                                                          #
#   File                   Contents                               Guide    #
#   ---------------------------------------------------------------------  #
#   khe_workload.c         Matchings: workload demand nodes        7.4     #
#   khe_grouping.c         Monitor grouping: helper functions      8.5     #
#   ---------------------------------------------------------------------  #
#   khe_vizier.c           Layer tree solvers: vizier nodes        9.2     #
#   khe_layer_solve.c      Layer tree solvers: miscellaneous       9.3-5   #
#   khe_merge_meets.c      Layer tree solvers: meet merging        9.6     #
#   ---------------------------------------------------------------------  #
#   khe_assign_time.c      Time solvers: miscellaneous            10.2-9   #
#   khe_runaround_time.c   Time solvers: runarounds               10.4     #
#   khe_kempe_move.c       Time solvers: Kempe meet moves         10.5     #
#   khe_layer_time.c       Time solvers: layered time assignment  10.6     #
#   khe_layer_match.c      Time solvers: layer matching           10.7     #
#   khe_zone_repair.c      Time solvers: zone repair              10.8.1   #
#   khe_layer_node_match.c Time solvers: layer node matching      10.8.2   #
#   khe_global_time.c      Time solvers: global time swaps        10.8.4   #
#   khe_simple_repair.c    Time solvers: simple repair            10.?     #
#   khe_time_repair.c      Time solvers: ejection chain repair    10.8.5   #
#   ---------------------------------------------------------------------  #
#   khe_assign_resource.c  Resource solvers: miscellaneous        11.2-10  #
#   khe_task_tree.c        Resource solvers: task trees           11.3     #
#   khe_task_group.c       Resource solvers: task groups          11.4     #
#   khe_first_resource.c   Resource solvers: most constrained     11.5     #
#   khe_pack_resource.c    Resource solvers: resource packing     11.6     #
#   khe_split_resource.c   Resource solvers: split assignments    11.7     #
#   khe_two_resource.c     Resource solvers: two-colour repair    11.7     #
#   khe_resource_repair.c  Resource solvers: ejection chainn rpr  11.7     #
#   ---------------------------------------------------------------------  #
#   khe_parallel_solve.c   General solving: parallel solving      12.1     #
#   khe_general_solve.c    General solving: miscellaneous         12.2     #
#   ---------------------------------------------------------------------  #
#   khe_ejector.c          Ejection chains: ejectors              13.2     #
#   khe_augment.c          Ejection chains: augment functions     13.3     #
#   ---------------------------------------------------------------------  #
#                                                                          #
############################################################################

SOLVER_HDRS = khe.h m.h

SOLVER_OBJS = khe_workload.o khe_grouping.o khe_vizier.o		\
  khe_layer_solve.o khe_merge_meets.o khe_assign_time.o			\
  khe_runaround_time.o khe_layer_match.o khe_layer_time.o		\
  khe_zone_repair.o khe_layer_node_match.o khe_global_time.o		\
  khe_simple_repair.o khe_kempe_move.o khe_time_repair.o		\
  khe_assign_resource.o khe_task_tree.o khe_task_group.o		\
  khe_first_resource.o khe_pack_resource.o khe_split_resource.o		\
  khe_two_resource.o khe_resource_repair.o khe_parallel_solve.o		\
  khe_general_solve.o khe_ejector.o khe_augment.o


############################################################################
#                                                                          #
#   Main rule, for producing binary "khe"                                  #
#                                                                          #
#   File khe_main.c contains a main() function, and invoking this rule     #
#   produces a binary which calls that function.  You can always throw     #
#   away this binary and khe_main.o and link the other .o files to your    #
#   own code.                                                              #
#                                                                          #
############################################################################

khe : $(SEPARATE_OBJS) $(ORDINARY_OBJS) $(LAYER_TREE_OBJS) $(SOLVER_OBJS) \
      khe_main.o
	$(CC) -o khe $(SEPARATE_OBJS) $(ORDINARY_OBJS) $(LAYER_TREE_OBJS) \
	$(SOLVER_OBJS) khe_main.o -lexpat -lm -pthread


############################################################################
#                                                                          #
#   Rules for separate files                                               #
#                                                                          #
#   Each of these can be compiled separately from the others and from      #
#   KHE generally, except that some use m.h.                               #
#                                                                          #
############################################################################

m.o : m.h

kml.o : kml.h m.h

khe_lset.o : khe_lset.h

khe_wmatch.o : khe_wmatch.h m.h

khe_partition.o : khe_partition.h m.h

khe_priqueue.o : khe_priqueue.h


############################################################################
#                                                                          #
#   Rules for ordinary, layer tree, solver, and khe_main.o files           #
#                                                                          #
#   See the comments above for an explanation of ordinary, layer tree,     #
#   and solver files.  File khe_main.o is basically a solver, but it       #
#   includes options for testing various things, so is a bit different.    #
#                                                                          #
############################################################################

$(ORDINARY_OBJS) : $(ORDINARY_HDRS)

$(LAYER_TREE_OBJS) : $(LAYER_TREE_HDRS)

$(SOLVER_OBJS) : $(SOLVER_HDRS)

khe_main.o : khe.h khe_partition.h khe_lset.h


############################################################################
#                                                                          #
#   Make clean (also cleans up doc subdirectory)                           #
#                                                                          #
############################################################################

clean:	
	-rm -f *.o khe
	-rm -fr doc/lout.li doc/*.ld doc/op*
