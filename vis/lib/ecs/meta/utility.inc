export namespace vis::ecs {
using vis::ecs::meta_dispatch;
using vis::ecs::meta_function_descriptor;
using vis::ecs::meta_function_descriptor_traits;
using vis::ecs::meta_function_helper;
using vis::ecs::meta_function_helper_t;

// TODO: I cannot export ::vis::ecs::meta_args because the function is declared as static. Is this a problem?
// error: using declaration referring to 'meta_arg' with internal linkage cannot be exported
// using vis::ecs::meta_arg;
using vis::ecs::meta_construct;
using vis::ecs::meta_getter;
using vis::ecs::meta_invoke;
using vis::ecs::meta_setter;
} // namespace vis::ecs