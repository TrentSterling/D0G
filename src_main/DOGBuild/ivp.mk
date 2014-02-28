DOG_C_DEFINES := \
  LINUX \
  IVP_VERSION_SDK \
  HAVANA_CONSTRAINTS
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/ivp/ivp_collision" \
  "$(DOG_SRC_MAIN)/ivp/ivp_compact_builder" \
  "$(DOG_SRC_MAIN)/ivp/ivp_controller" \
  "$(DOG_SRC_MAIN)/ivp/ivp_intern" \
  "$(DOG_SRC_MAIN)/ivp/ivp_physics" \
  "$(DOG_SRC_MAIN)/ivp/ivp_surface_manager" \
  "$(DOG_SRC_MAIN)/ivp/ivp_utility" \
  "$(DOG_SRC_MAIN)/ivp/havana/havok" \
  "$(DOG_SRC_MAIN)/ivp/havana"
DOG_SRC_FILES := \
  ivp/ivp_collision/ivp_3d_solver.cxx \
  ivp/ivp_collision/ivp_clustering_longrange.cxx \
  ivp/ivp_collision/ivp_clustering_lrange_hash.cxx \
  ivp/ivp_collision/ivp_clustering_visual_hash.cxx \
  ivp/ivp_collision/ivp_clustering_visualizer.cxx \
  ivp/ivp_collision/ivp_coll_del_root_mindist.cxx \
  ivp/ivp_collision/ivp_collision_filter.cxx \
  ivp/ivp_collision/ivp_compact_ledge.cxx \
  ivp/ivp_collision/ivp_compact_ledge_solver.cxx \
  ivp/ivp_collision/ivp_i_collision_vhash.cxx \
  ivp/ivp_collision/ivp_mindist.cxx \
  ivp/ivp_collision/ivp_mindist_debug.cxx \
  ivp/ivp_collision/ivp_mindist_event.cxx \
  ivp/ivp_collision/ivp_mindist_mcases.cxx \
  ivp/ivp_collision/ivp_mindist_minimize.cxx \
  ivp/ivp_collision/ivp_mindist_recursive.cxx \
  ivp/ivp_collision/ivp_oo_watcher.cxx \
  ivp/ivp_collision/ivp_range_manager.cxx \
  ivp/ivp_collision/ivp_ray_solver.cxx \
  ivp/ivp_compact_builder/geompack_cutfac.cxx \
  ivp/ivp_compact_builder/geompack_cvdec3.cxx \
  ivp/ivp_compact_builder/geompack_drdec3.cxx \
  ivp/ivp_compact_builder/geompack_dsphdc.cxx \
  ivp/ivp_compact_builder/geompack_edght.cxx \
  ivp/ivp_compact_builder/geompack_initcb.cxx \
  ivp/ivp_compact_builder/geompack_insed3.cxx \
  ivp/ivp_compact_builder/geompack_insfac.cxx \
  ivp/ivp_compact_builder/geompack_insvr3.cxx \
  ivp/ivp_compact_builder/geompack_prime.cxx \
  ivp/ivp_compact_builder/geompack_ptpolg.cxx \
  ivp/ivp_compact_builder/geompack_resedg.cxx \
  ivp/ivp_compact_builder/ivp_compact_ledge_gen.cxx \
  ivp/ivp_compact_builder/ivp_compact_modify.cxx \
  ivp/ivp_compact_builder/ivp_compact_recursive.cxx \
  ivp/ivp_compact_builder/ivp_convex_decompositor.cxx \
  ivp/ivp_compact_builder/ivp_halfspacesoup.cxx \
  ivp/ivp_compact_builder/ivp_i_fpoint_vhash.cxx \
  ivp/ivp_compact_builder/ivp_i_point_vhash.cxx \
  ivp/ivp_compact_builder/ivp_object_polygon_tetra.cxx \
  ivp/ivp_compact_builder/ivp_rot_inertia_solver.cxx \
  ivp/ivp_compact_builder/ivp_surbuild_halfspacesoup.cxx \
  ivp/ivp_compact_builder/ivp_surbuild_ledge_soup.cxx \
  ivp/ivp_compact_builder/ivp_surbuild_pointsoup.cxx \
  ivp/ivp_compact_builder/ivp_surbuild_polygon_convex.cxx \
  ivp/ivp_compact_builder/ivp_surbuild_polyhdrn_cncv.cxx \
  ivp/ivp_compact_builder/ivp_surbuild_q12.cxx \
  ivp/ivp_compact_builder/ivp_templates_intern.cxx \
  ivp/ivp_compact_builder/ivp_tetra_intrude.cxx \
  ivp/ivp_compact_builder/ivv_cluster_min_hash.cxx \
  ivp/ivp_compact_builder/qhull.cxx \
  ivp/ivp_compact_builder/qhull_geom.cxx \
  ivp/ivp_compact_builder/qhull_geom2.cxx \
  ivp/ivp_compact_builder/qhull_global.cxx \
  ivp/ivp_compact_builder/qhull_io.cxx \
  ivp/ivp_compact_builder/qhull_mem.cxx \
  ivp/ivp_compact_builder/qhull_merge.cxx \
  ivp/ivp_compact_builder/qhull_poly.cxx \
  ivp/ivp_compact_builder/qhull_poly2.cxx \
  ivp/ivp_compact_builder/qhull_qset.cxx \
  ivp/ivp_compact_builder/qhull_stat.cxx \
  ivp/ivp_compact_builder/qhull_user.cxx \
  ivp/ivp_controller/ivp_actuator.cxx \
  ivp/ivp_controller/ivp_actuator_spring.cxx \
  ivp/ivp_controller/ivp_buoyancy_solver.cxx \
  ivp/ivp_controller/ivp_car_system.cxx \
  ivp/ivp_controller/ivp_constraint.cxx \
  ivp/ivp_controller/ivp_constraint_car.cxx \
  ivp/ivp_controller/ivp_constraint_fixed_keyed.cxx \
  ivp/ivp_controller/ivp_constraint_local.cxx \
  ivp/ivp_controller/ivp_controller_buoyancy.cxx \
  ivp/ivp_controller/ivp_controller_floating.cxx \
  ivp/ivp_controller/ivp_controller_motion.cxx \
  ivp/ivp_controller/ivp_controller_raycast_car.cxx \
  ivp/ivp_controller/ivp_controller_stiff_spring.cxx \
  ivp/ivp_controller/ivp_controller_world_frict.cxx \
  ivp/ivp_controller/ivp_forcefield.cxx \
  ivp/ivp_controller/ivp_multidimensional_interp.cxx \
  ivp/ivp_controller/ivp_template_constraint.cxx \
  ivp/ivp_intern/ivp_ball.cxx \
  ivp/ivp_intern/ivp_calc_next_psi_solver.cxx \
  ivp/ivp_intern/ivp_controller_phantom.cxx \
  ivp/ivp_intern/ivp_core.cxx \
  ivp/ivp_intern/ivp_environment.cxx \
  ivp/ivp_intern/ivp_friction.cxx \
  ivp/ivp_intern/ivp_friction_gaps.cxx \
  ivp/ivp_intern/ivp_great_matrix.cxx \
  ivp/ivp_intern/ivp_hull_manager.cxx \
  ivp/ivp_intern/ivp_i_friction_hash.cxx \
  ivp/ivp_intern/ivp_i_object_vhash.cxx \
  ivp/ivp_intern/ivp_impact.cxx \
  ivp/ivp_intern/ivp_merge_core.cxx \
  ivp/ivp_intern/ivp_mindist_friction.cxx \
  ivp/ivp_intern/ivp_object.cxx \
  ivp/ivp_intern/ivp_object_attach.cxx \
  ivp/ivp_intern/ivp_physic.cxx \
  ivp/ivp_intern/ivp_physic_private.cxx \
  ivp/ivp_intern/ivp_polygon.cxx \
  ivp/ivp_intern/ivp_sim_unit.cxx \
  ivp/ivp_intern/ivp_solver_core_reaction.cxx \
  ivp/ivp_intern/ivp_time.cxx \
  ivp/ivp_physics/ivp_anomaly_manager.cxx \
  ivp/ivp_physics/ivp_betterdebugmanager.cxx \
  ivp/ivp_physics/ivp_betterstatisticsmanager.cxx \
  ivp/ivp_physics/ivp_cache_object.cxx \
  ivp/ivp_physics/ivp_liquid_surface_descript.cxx \
  ivp/ivp_physics/ivp_material.cxx \
  ivp/ivp_physics/ivp_performancecounter.cxx \
  ivp/ivp_physics/ivp_stat_manager_cback_con.cxx \
  ivp/ivp_physics/ivp_surface_manager.cxx \
  ivp/ivp_physics/ivp_templates.cxx \
  ivp/ivp_surface_manager/ivp_compact_surface.cxx \
  ivp/ivp_surface_manager/ivp_gridbuild_array.cxx \
  ivp/ivp_surface_manager/ivp_surman_grid.cxx \
  ivp/ivp_surface_manager/ivp_surman_polygon.cxx \
  ivp/ivp_utility/ivu_active_value.cxx \
  ivp/ivp_utility/ivu_bigvector.cxx \
  ivp/ivp_utility/ivu_geometry.cxx \
  ivp/ivp_utility/ivu_hash.cxx \
  ivp/ivp_utility/ivu_linear.cxx \
  ivp/ivp_utility/ivu_memory.cxx \
  ivp/ivp_utility/ivu_min_hash.cxx \
  ivp/ivp_utility/ivu_min_list.cxx \
  ivp/ivp_utility/ivu_os_dep.cxx \
  ivp/ivp_utility/ivu_quat.cxx \
  ivp/ivp_utility/ivu_string.cxx \
  ivp/ivp_utility/ivu_types.cxx \
  ivp/ivp_utility/ivu_vector.cxx \
  ivp/ivp_utility/ivu_vhash.cxx \
  ivp/havana/havok/hk_base/array/array.cpp \
  ivp/havana/havok/hk_base/hash/hash.cpp \
  ivp/havana/havok/hk_base/id_server/id_server.cpp \
  ivp/havana/havok/hk_base/memory/memory.cpp \
  ivp/havana/havok/hk_base/memory/memory_util.cpp \
  ivp/havana/havok/hk_base/stopwatch/stopwatch.cpp \
  ivp/havana/havok/hk_base/string/string.cpp \
  ivp/havana/havok/hk_base/base_types.cpp \
  ivp/havana/havok/hk_base/console.cpp \
  ivp/havana/havok/hk_math/lcp/lcp_solver.cpp \
  ivp/havana/havok/hk_math/incr_lu/incr_lu.cpp \
  ivp/havana/havok/hk_math/gauss_elimination/gauss_elimination.cpp \
  ivp/havana/havok/hk_math/quaternion/quaternion.cpp \
  ivp/havana/havok/hk_math/quaternion/quaternion_util.cpp \
  ivp/havana/havok/hk_math/vector3/vector3.cpp \
  ivp/havana/havok/hk_math/vector3/vector3_util.cpp \
  ivp/havana/havok/hk_math/densematrix.cpp \
  ivp/havana/havok/hk_math/densematrix_util.cpp \
  ivp/havana/havok/hk_math/eulerangles.cpp \
  ivp/havana/havok/hk_math/math.cpp \
  ivp/havana/havok/hk_math/matrix3.cpp \
  ivp/havana/havok/hk_math/odesolve.cpp \
  ivp/havana/havok/hk_math/plane.cpp \
  ivp/havana/havok/hk_math/rotation.cpp \
  ivp/havana/havok/hk_math/spatial_matrix.cpp \
  ivp/havana/havok/hk_math/spatial_vector.cpp \
  ivp/havana/havok/hk_math/transform.cpp \
  ivp/havana/havok/hk_physics/constraint/ball_socket/ball_socket_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/breakable_constraint/breakable_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/fixed/fixed_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/hinge/hinge_bp_builder.cpp \
  ivp/havana/havok/hk_physics/constraint/hinge/hinge_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/limited_ball_socket/limited_ball_socket_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/prismatic/prismatic_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/pulley/pulley_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/ragdoll/ragdoll_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/ragdoll/ragdoll_constraint_bp_builder.cpp \
  ivp/havana/havok/hk_physics/constraint/stiff_spring/stiff_spring_constraint.cpp \
  ivp/havana/havok/hk_physics/constraint/constraint.cpp \
  ivp/ivp_physics/hk_physics/constraint/local_constraint_system/local_constraint_system.cpp \
  ivp/ivp_physics/hk_physics/core/rigid_body_core.cpp \
  ivp/ivp_physics/hk_physics/effector/rigid_body_binary_effector.cpp
DOG_STATIC_LIBRARY := true