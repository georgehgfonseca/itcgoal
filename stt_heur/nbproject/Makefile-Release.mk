#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include stt_heur-Makefile.mk

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/stt_heur/khe/khe_archive.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_events_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_prefer_resources_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_soln_group.o \
	${OBJECTDIR}/stt_heur/khe/khe_transaction.o \
	${OBJECTDIR}/stt_heur/khe/khe_zone_repair.o \
	${OBJECTDIR}/stt_heur/khe/khe_pack_resource.o \
	${OBJECTDIR}/stt_heur/khe/khe_link_events_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_workload.o \
	${OBJECTDIR}/stt_heur/khe/khe_runaround_time.o \
	${OBJECTDIR}/stt_heur/khe/khe_soln_group_metadata.o \
	${OBJECTDIR}/stt_heur/khe/khe_avoid_unavailable_times_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_dev_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_priqueue.o \
	${OBJECTDIR}/stt_heur/khe/khe_avoid_unavailable_times_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_distribute_split_events_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_assign_resource_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_timetable_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_time_group_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_trace.o \
	${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_cluster_busy_times_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_spread_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_prefer_times_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_prefer_times_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_event.o \
	${OBJECTDIR}/stt_heur/khe/khe_resource.o \
	${OBJECTDIR}/stt_heur/khe/khe_task_group.o \
	${OBJECTDIR}/stt_heur/khe/khe_distribute_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_instance.o \
	${OBJECTDIR}/stt_heur/khe/khe_evenness_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_parallel_solve.o \
	${OBJECTDIR}/stt_heur/khe/khe_event_group.o \
	${OBJECTDIR}/stt_heur/khe/khe_instance_metadata.o \
	${OBJECTDIR}/stt_heur/khe/khe_limit_workload_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_kempe_move.o \
	${OBJECTDIR}/stt_heur/khe/khe_resource_group.o \
	${OBJECTDIR}/stt_heur/khe/khe_general_solve.o \
	${OBJECTDIR}/stt_heur/khe/khe_ordinary_demand_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_time_group.o \
	${OBJECTDIR}/stt_heur/khe/khe_vizier.o \
	${OBJECTDIR}/stt_heur/khe/khe_zone.o \
	${OBJECTDIR}/stt_heur/khe/khe_two_resource.o \
	${OBJECTDIR}/stt_heur/khe/khe_assign_time_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_forest.o \
	${OBJECTDIR}/stt_heur/khe/khe_limit_busy_times_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_soln.o \
	${OBJECTDIR}/stt_heur/khe/khe_spread_events_constraint.o \
	${OBJECTDIR}/stt_heur/config.o \
	${OBJECTDIR}/stt_heur/khe/khe_lset.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_events_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_evenness_handler.o \
	${OBJECTDIR}/stt_heur/heuristics.o \
	${OBJECTDIR}/stt_heur/khe/khe_global_time.o \
	${OBJECTDIR}/stt_heur/khe/khe_distribute_split_events_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_meet.o \
	${OBJECTDIR}/stt_heur/khe/khe_merge_meets.o \
	${OBJECTDIR}/stt_heur/khe/khe_assigned_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_assign_resource_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_node.o \
	${OBJECTDIR}/stt_heur/khe/khe_avoid_split_assignments_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_resource_in_soln.o \
	${OBJECTDIR}/stt_heur/khe/khe_time.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_class.o \
	${OBJECTDIR}/stt_heur/khe/khe_layer_node_match.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_layer.o \
	${OBJECTDIR}/stt_heur/khe/khe_domain_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_time_group_nhood.o \
	${OBJECTDIR}/stt_heur/khe/khe_link_events_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_resource_repair.o \
	${OBJECTDIR}/stt_heur/khe/kml.o \
	${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_assign_time.o \
	${OBJECTDIR}/stt_heur/khe/khe_workload_demand_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_prefer_resources_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_cycle_meet.o \
	${OBJECTDIR}/stt_heur/khe/khe_pack_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_partition.o \
	${OBJECTDIR}/stt_heur/khe/khe_event_resource_group.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_ejector.o \
	${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_assign_resource.o \
	${OBJECTDIR}/stt_heur/khe/khe_cluster_busy_times_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_grouping.o \
	${OBJECTDIR}/stt_heur/khe/khe_assign_time_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_wmatch.o \
	${OBJECTDIR}/stt_heur/khe/khe_archive_metadata.o \
	${OBJECTDIR}/stt_heur/khe/khe_time_repair.o \
	${OBJECTDIR}/stt_heur/khe/khe_link_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_layer_match.o \
	${OBJECTDIR}/stt_heur/khe/khe_augment.o \
	${OBJECTDIR}/stt_heur/khe/khe_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_resource.o \
	${OBJECTDIR}/stt_heur/khe/khe_limit_busy_times_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_limit_idle_times_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_event_resource_in_soln.o \
	${OBJECTDIR}/stt_heur/khe/m.o \
	${OBJECTDIR}/stt_heur/khe/khe_group_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_preassigned_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_simple_repair.o \
	${OBJECTDIR}/stt_heur/khe/khe_tasking.o \
	${OBJECTDIR}/stt_heur/khe/khe_split_task.o \
	${OBJECTDIR}/stt_heur/khe/khe_event_in_soln.o \
	${OBJECTDIR}/stt_heur/khe/khe_layer.o \
	${OBJECTDIR}/stt_heur/khe/khe_layer_time.o \
	${OBJECTDIR}/stt_heur/khe/khe_avoid_split_assignments_constraint.o \
	${OBJECTDIR}/stt_heur/main.o \
	${OBJECTDIR}/stt_heur/khe/khe_limit_workload_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_limit_idle_times_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_constraint.o \
	${OBJECTDIR}/stt_heur/khe/khe_event_resource.o \
	${OBJECTDIR}/stt_heur/khe/khe_first_resource.o \
	${OBJECTDIR}/stt_heur/khe/khe_layer_solve.o \
	${OBJECTDIR}/stt_heur/moves.o \
	${OBJECTDIR}/stt_heur/khe/khe_task_tree.o \
	${OBJECTDIR}/stt_heur/khe/khe_spread_events_monitor.o \
	${OBJECTDIR}/stt_heur/khe/khe_resource_type.o \
	${OBJECTDIR}/stt_heur/khe/khe_layer_tree.o \
	${OBJECTDIR}/stt_heur/khe/khe_matching.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lexpat -lpthread -lm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stt_heur

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stt_heur: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stt_heur ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/stt_heur/khe/khe_archive.o: stt_heur/khe/khe_archive.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_archive.o stt_heur/khe/khe_archive.c

${OBJECTDIR}/stt_heur/khe/khe_split_events_constraint.o: stt_heur/khe/khe_split_events_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_events_constraint.o stt_heur/khe/khe_split_events_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_prefer_resources_constraint.o: stt_heur/khe/khe_prefer_resources_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_prefer_resources_constraint.o stt_heur/khe/khe_prefer_resources_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_soln_group.o: stt_heur/khe/khe_soln_group.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_soln_group.o stt_heur/khe/khe_soln_group.c

${OBJECTDIR}/stt_heur/khe/khe_transaction.o: stt_heur/khe/khe_transaction.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_transaction.o stt_heur/khe/khe_transaction.c

${OBJECTDIR}/stt_heur/khe/khe_zone_repair.o: stt_heur/khe/khe_zone_repair.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_zone_repair.o stt_heur/khe/khe_zone_repair.c

${OBJECTDIR}/stt_heur/khe/khe_pack_resource.o: stt_heur/khe/khe_pack_resource.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_pack_resource.o stt_heur/khe/khe_pack_resource.c

${OBJECTDIR}/stt_heur/khe/khe_link_events_monitor.o: stt_heur/khe/khe_link_events_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_link_events_monitor.o stt_heur/khe/khe_link_events_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_workload.o: stt_heur/khe/khe_workload.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_workload.o stt_heur/khe/khe_workload.c

${OBJECTDIR}/stt_heur/khe/khe_runaround_time.o: stt_heur/khe/khe_runaround_time.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_runaround_time.o stt_heur/khe/khe_runaround_time.c

${OBJECTDIR}/stt_heur/khe/khe_soln_group_metadata.o: stt_heur/khe/khe_soln_group_metadata.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_soln_group_metadata.o stt_heur/khe/khe_soln_group_metadata.c

${OBJECTDIR}/stt_heur/khe/khe_avoid_unavailable_times_monitor.o: stt_heur/khe/khe_avoid_unavailable_times_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_avoid_unavailable_times_monitor.o stt_heur/khe/khe_avoid_unavailable_times_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_dev_monitor.o: stt_heur/khe/khe_dev_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_dev_monitor.o stt_heur/khe/khe_dev_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_priqueue.o: stt_heur/khe/khe_priqueue.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_priqueue.o stt_heur/khe/khe_priqueue.c

${OBJECTDIR}/stt_heur/khe/khe_avoid_unavailable_times_constraint.o: stt_heur/khe/khe_avoid_unavailable_times_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_avoid_unavailable_times_constraint.o stt_heur/khe/khe_avoid_unavailable_times_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_distribute_split_events_constraint.o: stt_heur/khe/khe_distribute_split_events_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_distribute_split_events_constraint.o stt_heur/khe/khe_distribute_split_events_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_assign_resource_constraint.o: stt_heur/khe/khe_assign_resource_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_assign_resource_constraint.o stt_heur/khe/khe_assign_resource_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_timetable_monitor.o: stt_heur/khe/khe_timetable_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_timetable_monitor.o stt_heur/khe/khe_timetable_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_time_group_monitor.o: stt_heur/khe/khe_time_group_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_time_group_monitor.o stt_heur/khe/khe_time_group_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_trace.o: stt_heur/khe/khe_trace.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_trace.o stt_heur/khe/khe_trace.c

${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_constraint.o: stt_heur/khe/khe_avoid_clashes_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_constraint.o stt_heur/khe/khe_avoid_clashes_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_monitor.o: stt_heur/khe/khe_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_monitor.o stt_heur/khe/khe_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_cluster_busy_times_monitor.o: stt_heur/khe/khe_cluster_busy_times_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_cluster_busy_times_monitor.o stt_heur/khe/khe_cluster_busy_times_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_spread_split_task.o: stt_heur/khe/khe_spread_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_spread_split_task.o stt_heur/khe/khe_spread_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_prefer_times_monitor.o: stt_heur/khe/khe_prefer_times_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_prefer_times_monitor.o stt_heur/khe/khe_prefer_times_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_prefer_times_constraint.o: stt_heur/khe/khe_prefer_times_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_prefer_times_constraint.o stt_heur/khe/khe_prefer_times_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_event.o: stt_heur/khe/khe_event.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_event.o stt_heur/khe/khe_event.c

${OBJECTDIR}/stt_heur/khe/khe_resource.o: stt_heur/khe/khe_resource.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_resource.o stt_heur/khe/khe_resource.c

${OBJECTDIR}/stt_heur/khe/khe_task_group.o: stt_heur/khe/khe_task_group.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_task_group.o stt_heur/khe/khe_task_group.c

${OBJECTDIR}/stt_heur/khe/khe_distribute_split_task.o: stt_heur/khe/khe_distribute_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_distribute_split_task.o stt_heur/khe/khe_distribute_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_instance.o: stt_heur/khe/khe_instance.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_instance.o stt_heur/khe/khe_instance.c

${OBJECTDIR}/stt_heur/khe/khe_evenness_monitor.o: stt_heur/khe/khe_evenness_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_evenness_monitor.o stt_heur/khe/khe_evenness_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_parallel_solve.o: stt_heur/khe/khe_parallel_solve.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_parallel_solve.o stt_heur/khe/khe_parallel_solve.c

${OBJECTDIR}/stt_heur/khe/khe_event_group.o: stt_heur/khe/khe_event_group.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_event_group.o stt_heur/khe/khe_event_group.c

${OBJECTDIR}/stt_heur/khe/khe_instance_metadata.o: stt_heur/khe/khe_instance_metadata.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_instance_metadata.o stt_heur/khe/khe_instance_metadata.c

${OBJECTDIR}/stt_heur/khe/khe_limit_workload_monitor.o: stt_heur/khe/khe_limit_workload_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_limit_workload_monitor.o stt_heur/khe/khe_limit_workload_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_kempe_move.o: stt_heur/khe/khe_kempe_move.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_kempe_move.o stt_heur/khe/khe_kempe_move.c

${OBJECTDIR}/stt_heur/khe/khe_resource_group.o: stt_heur/khe/khe_resource_group.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_resource_group.o stt_heur/khe/khe_resource_group.c

${OBJECTDIR}/stt_heur/khe/khe_general_solve.o: stt_heur/khe/khe_general_solve.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_general_solve.o stt_heur/khe/khe_general_solve.c

${OBJECTDIR}/stt_heur/khe/khe_ordinary_demand_monitor.o: stt_heur/khe/khe_ordinary_demand_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_ordinary_demand_monitor.o stt_heur/khe/khe_ordinary_demand_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_time_group.o: stt_heur/khe/khe_time_group.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_time_group.o stt_heur/khe/khe_time_group.c

${OBJECTDIR}/stt_heur/khe/khe_vizier.o: stt_heur/khe/khe_vizier.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_vizier.o stt_heur/khe/khe_vizier.c

${OBJECTDIR}/stt_heur/khe/khe_zone.o: stt_heur/khe/khe_zone.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_zone.o stt_heur/khe/khe_zone.c

${OBJECTDIR}/stt_heur/khe/khe_two_resource.o: stt_heur/khe/khe_two_resource.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_two_resource.o stt_heur/khe/khe_two_resource.c

${OBJECTDIR}/stt_heur/khe/khe_assign_time_constraint.o: stt_heur/khe/khe_assign_time_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_assign_time_constraint.o stt_heur/khe/khe_assign_time_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_split_forest.o: stt_heur/khe/khe_split_forest.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_forest.o stt_heur/khe/khe_split_forest.c

${OBJECTDIR}/stt_heur/khe/khe_limit_busy_times_constraint.o: stt_heur/khe/khe_limit_busy_times_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_limit_busy_times_constraint.o stt_heur/khe/khe_limit_busy_times_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_soln.o: stt_heur/khe/khe_soln.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_soln.o stt_heur/khe/khe_soln.c

${OBJECTDIR}/stt_heur/khe/khe_spread_events_constraint.o: stt_heur/khe/khe_spread_events_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_spread_events_constraint.o stt_heur/khe/khe_spread_events_constraint.c

${OBJECTDIR}/stt_heur/config.o: stt_heur/config.cpp 
	${MKDIR} -p ${OBJECTDIR}/stt_heur
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/config.o stt_heur/config.cpp

${OBJECTDIR}/stt_heur/khe/khe_lset.o: stt_heur/khe/khe_lset.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_lset.o stt_heur/khe/khe_lset.c

${OBJECTDIR}/stt_heur/khe/khe_split_events_monitor.o: stt_heur/khe/khe_split_events_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_events_monitor.o stt_heur/khe/khe_split_events_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_evenness_handler.o: stt_heur/khe/khe_evenness_handler.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_evenness_handler.o stt_heur/khe/khe_evenness_handler.c

${OBJECTDIR}/stt_heur/heuristics.o: stt_heur/heuristics.cpp 
	${MKDIR} -p ${OBJECTDIR}/stt_heur
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/heuristics.o stt_heur/heuristics.cpp

${OBJECTDIR}/stt_heur/khe/khe_global_time.o: stt_heur/khe/khe_global_time.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_global_time.o stt_heur/khe/khe_global_time.c

${OBJECTDIR}/stt_heur/khe/khe_distribute_split_events_monitor.o: stt_heur/khe/khe_distribute_split_events_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_distribute_split_events_monitor.o stt_heur/khe/khe_distribute_split_events_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_meet.o: stt_heur/khe/khe_meet.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_meet.o stt_heur/khe/khe_meet.c

${OBJECTDIR}/stt_heur/khe/khe_merge_meets.o: stt_heur/khe/khe_merge_meets.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_merge_meets.o stt_heur/khe/khe_merge_meets.c

${OBJECTDIR}/stt_heur/khe/khe_assigned_split_task.o: stt_heur/khe/khe_assigned_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_assigned_split_task.o stt_heur/khe/khe_assigned_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_assign_resource_monitor.o: stt_heur/khe/khe_assign_resource_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_assign_resource_monitor.o stt_heur/khe/khe_assign_resource_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_node.o: stt_heur/khe/khe_node.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_node.o stt_heur/khe/khe_node.c

${OBJECTDIR}/stt_heur/khe/khe_avoid_split_assignments_monitor.o: stt_heur/khe/khe_avoid_split_assignments_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_avoid_split_assignments_monitor.o stt_heur/khe/khe_avoid_split_assignments_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_resource_in_soln.o: stt_heur/khe/khe_resource_in_soln.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_resource_in_soln.o stt_heur/khe/khe_resource_in_soln.c

${OBJECTDIR}/stt_heur/khe/khe_time.o: stt_heur/khe/khe_time.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_time.o stt_heur/khe/khe_time.c

${OBJECTDIR}/stt_heur/khe/khe_split_class.o: stt_heur/khe/khe_split_class.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_class.o stt_heur/khe/khe_split_class.c

${OBJECTDIR}/stt_heur/khe/khe_layer_node_match.o: stt_heur/khe/khe_layer_node_match.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_layer_node_match.o stt_heur/khe/khe_layer_node_match.c

${OBJECTDIR}/stt_heur/khe/khe_split_layer.o: stt_heur/khe/khe_split_layer.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_layer.o stt_heur/khe/khe_split_layer.c

${OBJECTDIR}/stt_heur/khe/khe_domain_split_task.o: stt_heur/khe/khe_domain_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_domain_split_task.o stt_heur/khe/khe_domain_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_time_group_nhood.o: stt_heur/khe/khe_time_group_nhood.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_time_group_nhood.o stt_heur/khe/khe_time_group_nhood.c

${OBJECTDIR}/stt_heur/khe/khe_link_events_constraint.o: stt_heur/khe/khe_link_events_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_link_events_constraint.o stt_heur/khe/khe_link_events_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_resource_repair.o: stt_heur/khe/khe_resource_repair.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_resource_repair.o stt_heur/khe/khe_resource_repair.c

${OBJECTDIR}/stt_heur/khe/kml.o: stt_heur/khe/kml.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/kml.o stt_heur/khe/kml.c

${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_split_task.o: stt_heur/khe/khe_avoid_clashes_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_split_task.o stt_heur/khe/khe_avoid_clashes_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_assign_time.o: stt_heur/khe/khe_assign_time.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_assign_time.o stt_heur/khe/khe_assign_time.c

${OBJECTDIR}/stt_heur/khe/khe_workload_demand_monitor.o: stt_heur/khe/khe_workload_demand_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_workload_demand_monitor.o stt_heur/khe/khe_workload_demand_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_prefer_resources_monitor.o: stt_heur/khe/khe_prefer_resources_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_prefer_resources_monitor.o stt_heur/khe/khe_prefer_resources_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_cycle_meet.o: stt_heur/khe/khe_cycle_meet.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_cycle_meet.o stt_heur/khe/khe_cycle_meet.c

${OBJECTDIR}/stt_heur/khe/khe_pack_split_task.o: stt_heur/khe/khe_pack_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_pack_split_task.o stt_heur/khe/khe_pack_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_partition.o: stt_heur/khe/khe_partition.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_partition.o stt_heur/khe/khe_partition.c

${OBJECTDIR}/stt_heur/khe/khe_event_resource_group.o: stt_heur/khe/khe_event_resource_group.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_event_resource_group.o stt_heur/khe/khe_event_resource_group.c

${OBJECTDIR}/stt_heur/khe/khe_split_split_task.o: stt_heur/khe/khe_split_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_split_task.o stt_heur/khe/khe_split_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_ejector.o: stt_heur/khe/khe_ejector.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_ejector.o stt_heur/khe/khe_ejector.c

${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_monitor.o: stt_heur/khe/khe_avoid_clashes_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_avoid_clashes_monitor.o stt_heur/khe/khe_avoid_clashes_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_assign_resource.o: stt_heur/khe/khe_assign_resource.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_assign_resource.o stt_heur/khe/khe_assign_resource.c

${OBJECTDIR}/stt_heur/khe/khe_cluster_busy_times_constraint.o: stt_heur/khe/khe_cluster_busy_times_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_cluster_busy_times_constraint.o stt_heur/khe/khe_cluster_busy_times_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_grouping.o: stt_heur/khe/khe_grouping.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_grouping.o stt_heur/khe/khe_grouping.c

${OBJECTDIR}/stt_heur/khe/khe_assign_time_monitor.o: stt_heur/khe/khe_assign_time_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_assign_time_monitor.o stt_heur/khe/khe_assign_time_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_wmatch.o: stt_heur/khe/khe_wmatch.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_wmatch.o stt_heur/khe/khe_wmatch.c

${OBJECTDIR}/stt_heur/khe/khe_archive_metadata.o: stt_heur/khe/khe_archive_metadata.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_archive_metadata.o stt_heur/khe/khe_archive_metadata.c

${OBJECTDIR}/stt_heur/khe/khe_time_repair.o: stt_heur/khe/khe_time_repair.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_time_repair.o stt_heur/khe/khe_time_repair.c

${OBJECTDIR}/stt_heur/khe/khe_link_split_task.o: stt_heur/khe/khe_link_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_link_split_task.o stt_heur/khe/khe_link_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_layer_match.o: stt_heur/khe/khe_layer_match.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_layer_match.o stt_heur/khe/khe_layer_match.c

${OBJECTDIR}/stt_heur/khe/khe_augment.o: stt_heur/khe/khe_augment.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_augment.o stt_heur/khe/khe_augment.c

${OBJECTDIR}/stt_heur/khe/khe_task.o: stt_heur/khe/khe_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_task.o stt_heur/khe/khe_task.c

${OBJECTDIR}/stt_heur/khe/khe_split_resource.o: stt_heur/khe/khe_split_resource.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_resource.o stt_heur/khe/khe_split_resource.c

${OBJECTDIR}/stt_heur/khe/khe_limit_busy_times_monitor.o: stt_heur/khe/khe_limit_busy_times_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_limit_busy_times_monitor.o stt_heur/khe/khe_limit_busy_times_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_limit_idle_times_constraint.o: stt_heur/khe/khe_limit_idle_times_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_limit_idle_times_constraint.o stt_heur/khe/khe_limit_idle_times_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_event_resource_in_soln.o: stt_heur/khe/khe_event_resource_in_soln.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_event_resource_in_soln.o stt_heur/khe/khe_event_resource_in_soln.c

${OBJECTDIR}/stt_heur/khe/m.o: stt_heur/khe/m.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/m.o stt_heur/khe/m.c

${OBJECTDIR}/stt_heur/khe/khe_group_monitor.o: stt_heur/khe/khe_group_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_group_monitor.o stt_heur/khe/khe_group_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_preassigned_split_task.o: stt_heur/khe/khe_preassigned_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_preassigned_split_task.o stt_heur/khe/khe_preassigned_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_simple_repair.o: stt_heur/khe/khe_simple_repair.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_simple_repair.o stt_heur/khe/khe_simple_repair.c

${OBJECTDIR}/stt_heur/khe/khe_tasking.o: stt_heur/khe/khe_tasking.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_tasking.o stt_heur/khe/khe_tasking.c

${OBJECTDIR}/stt_heur/khe/khe_split_task.o: stt_heur/khe/khe_split_task.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_split_task.o stt_heur/khe/khe_split_task.c

${OBJECTDIR}/stt_heur/khe/khe_event_in_soln.o: stt_heur/khe/khe_event_in_soln.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_event_in_soln.o stt_heur/khe/khe_event_in_soln.c

${OBJECTDIR}/stt_heur/khe/khe_layer.o: stt_heur/khe/khe_layer.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_layer.o stt_heur/khe/khe_layer.c

${OBJECTDIR}/stt_heur/khe/khe_layer_time.o: stt_heur/khe/khe_layer_time.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_layer_time.o stt_heur/khe/khe_layer_time.c

${OBJECTDIR}/stt_heur/khe/khe_avoid_split_assignments_constraint.o: stt_heur/khe/khe_avoid_split_assignments_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_avoid_split_assignments_constraint.o stt_heur/khe/khe_avoid_split_assignments_constraint.c

${OBJECTDIR}/stt_heur/main.o: stt_heur/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/stt_heur
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/main.o stt_heur/main.cpp

${OBJECTDIR}/stt_heur/khe/khe_limit_workload_constraint.o: stt_heur/khe/khe_limit_workload_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_limit_workload_constraint.o stt_heur/khe/khe_limit_workload_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_limit_idle_times_monitor.o: stt_heur/khe/khe_limit_idle_times_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_limit_idle_times_monitor.o stt_heur/khe/khe_limit_idle_times_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_constraint.o: stt_heur/khe/khe_constraint.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_constraint.o stt_heur/khe/khe_constraint.c

${OBJECTDIR}/stt_heur/khe/khe_event_resource.o: stt_heur/khe/khe_event_resource.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_event_resource.o stt_heur/khe/khe_event_resource.c

${OBJECTDIR}/stt_heur/khe/khe_first_resource.o: stt_heur/khe/khe_first_resource.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_first_resource.o stt_heur/khe/khe_first_resource.c

${OBJECTDIR}/stt_heur/khe/khe_layer_solve.o: stt_heur/khe/khe_layer_solve.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_layer_solve.o stt_heur/khe/khe_layer_solve.c

${OBJECTDIR}/stt_heur/moves.o: stt_heur/moves.cpp 
	${MKDIR} -p ${OBJECTDIR}/stt_heur
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/moves.o stt_heur/moves.cpp

${OBJECTDIR}/stt_heur/khe/khe_task_tree.o: stt_heur/khe/khe_task_tree.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_task_tree.o stt_heur/khe/khe_task_tree.c

${OBJECTDIR}/stt_heur/khe/khe_spread_events_monitor.o: stt_heur/khe/khe_spread_events_monitor.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_spread_events_monitor.o stt_heur/khe/khe_spread_events_monitor.c

${OBJECTDIR}/stt_heur/khe/khe_resource_type.o: stt_heur/khe/khe_resource_type.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_resource_type.o stt_heur/khe/khe_resource_type.c

${OBJECTDIR}/stt_heur/khe/khe_layer_tree.o: stt_heur/khe/khe_layer_tree.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_layer_tree.o stt_heur/khe/khe_layer_tree.c

${OBJECTDIR}/stt_heur/khe/khe_matching.o: stt_heur/khe/khe_matching.c 
	${MKDIR} -p ${OBJECTDIR}/stt_heur/khe
	${RM} $@.d
	$(COMPILE.c) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stt_heur/khe/khe_matching.o stt_heur/khe/khe_matching.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stt_heur

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
