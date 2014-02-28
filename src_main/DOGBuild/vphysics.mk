DOG_C_DEFINES := \
  LINUX \
  HAVANA_CONSTRAINTS \
  VPHYSICS_EXPORTS
DOG_C_INCLUDES := \
  "$(DOG_SRC_MAIN)/ivp/ivp_collision" \
  "$(DOG_SRC_MAIN)/ivp/ivp_compact_builder" \
  "$(DOG_SRC_MAIN)/ivp/ivp_controller" \
  "$(DOG_SRC_MAIN)/ivp/ivp_intern" \
  "$(DOG_SRC_MAIN)/ivp/ivp_physics" \
  "$(DOG_SRC_MAIN)/ivp/ivp_surface_manager" \
  "$(DOG_SRC_MAIN)/ivp/ivp_utility" \
  "$(DOG_SRC_MAIN)/ivp/havana/havok" \
  "$(DOG_SRC_MAIN)/ivp/havana" \
  "$(DOG_SRC_MAIN)/public/mathlib" \
  "$(DOG_SRC_MAIN)/public/tier0" \
  "$(DOG_SRC_MAIN)/public/tier1"
DOG_PUBLIC_LIBRARIES := \
  ivp \
  mathlib \
  tier0 \
  tier1 \
  vstdlib
DOG_SRC_FILES := \
  ivp/ivp_controller/ivp_controller_airboat.cpp \
  vphysics/convert.cpp \
  public/filesystem_helpers.cpp \
  vphysics/main.cpp \
  public/tier0/memoverride.cpp \
  vphysics/physics_airboat.cpp \
  vphysics/physics_collide.cpp \
  vphysics/physics_collision_set.cpp \
  vphysics/physics_constraint.cpp \
  vphysics/physics_controller_raycast_vehicle.cpp \
  vphysics/physics_environment.cpp \
  vphysics/physics_fluid.cpp \
  vphysics/physics_friction.cpp \
  vphysics/physics_material.cpp \
  vphysics/physics_motioncontroller.cpp \
  vphysics/physics_object.cpp \
  vphysics/physics_object_hash.cpp \
  vphysics/physics_shadow.cpp \
  vphysics/physics_spring.cpp \
  vphysics/physics_vehicle.cpp \
  vphysics/trace.cpp \
  vphysics/vcollide_parse.cpp \
  vphysics/vphysics_saverestore.cpp