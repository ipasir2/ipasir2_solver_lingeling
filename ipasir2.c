#include "ipasir2.h"
#include "lglib.h"


ipasir2_errorcode ipasir2_signature(char const** signature) {
  *signature = "Lingeling 1.0.0";
  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_init(void** solver) {
  *solver = lglinit();
  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_release(void* solver) {
  lglrelease((LGL*)solver);
  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_options(void* solver, ipasir2_option const** options) {
  return IPASIR2_E_UNSUPPORTED;
}


ipasir2_errorcode ipasir2_set_option(void* solver, ipasir2_option const* handle, int64_t value, int64_t index) {
  return IPASIR2_E_UNSUPPORTED;
}


ipasir2_errorcode ipasir2_add(void* solver, int32_t const* clause, int32_t len, ipasir2_redundancy redundancy) {
  LGL* lingeling = (LGL*)solver;

  for(size_t idx = 0; idx < len; ++idx) {
    lgladd(lingeling, clause[idx]);
    lglfreeze(lingeling, clause[idx]);
  }
  lgladd(lingeling, 0);

  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_solve(void* solver, int* result, int32_t const* literals, int32_t len) {
  LGL* lingeling = (LGL*)solver;

  for(size_t idx = 0; idx < len; ++idx) {
    lglassume(lingeling, literals[idx]);
  }

  *result = lglsat(lingeling);

  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_val(void* solver, int32_t lit, int32_t* result) {
  LGL* lingeling = (LGL*)solver;

  int lgl_result = lglderef(solver, lit);
  *result = (lgl_result > 0 ? lit : -lit);

  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_failed(void* solver, int32_t lit, int* result) {
  LGL* lingeling = (LGL*)solver;

  *result = lglfailed(solver, lit) > 0 ? 1 : 0;

  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_set_terminate(void* solver, void* data, int (*callback)(void* data)) {
  LGL* lingeling = (LGL*)solver;

  lglseterm(lingeling, callback, data);

  return IPASIR2_E_OK;
}


ipasir2_errorcode ipasir2_set_export(void* solver, void* data, int max_length, void (*callback)(void* data, int32_t const* clause)) {
  return IPASIR2_E_UNSUPPORTED;
}


ipasir2_errorcode ipasir2_set_import(void* solver, void* data, ipasir2_redundancy pledge,
    void (*callback)(void* data, ipasir2_redundancy min)) {
  return IPASIR2_E_UNSUPPORTED;
}


ipasir2_errorcode ipasir2_set_notify(void* solver, void* data,
    void (*callback)(void* data, int32_t const* assigned, int32_t const* unassigned)) {
  return IPASIR2_E_UNSUPPORTED;
}
