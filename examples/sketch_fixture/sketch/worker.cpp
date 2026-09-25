/* SPDX-License-Identifier: Apache-2.0 */
#include "worker.hpp"
#include "detail/value.hpp"
#include "generated_resource.hpp"
int worker_value() { return SKETCH_NESTED_VALUE; }
int worker_resource() { return SKETCH_RESOURCE_VALUE; }
