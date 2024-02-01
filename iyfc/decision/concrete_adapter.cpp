#include "concrete_adapter.h"
#include <unordered_map>
#include "comm_include.h"
#include "alo_register.h"
#include "err_code.h"
#include "proto/iyfc.pb.h"

namespace iyfc {

int ConcreteAdapter::setParaAndSig(
    std::shared_ptr<ParametersInterface> ptr_parameters) {
  // Cast to the subclass pointer -- currently no specific parameter settings
  auto ptr_concret_param =
      std::static_pointer_cast<ConcreteParametersHandler>(ptr_parameters);
  // Get config
  return 0;
}

int ConcreteAdapter::genKeys(const string& alo_name, const Dag& dag) {
  m_concrete_ctx = concreteGenerateKeys();
  return 0;
}

int ConcreteAdapter::encrypt(const Valuation& inputs, bool replace) {
  auto& secret_ctx = std::get<1>(m_concrete_ctx);
  if (secret_ctx == nullptr) {
    throw std::logic_error("encrypt secret_ctx null !");
  }

  m_concrete_valution = secret_ctx->encrypt(inputs);
  return 0;
}

int ConcreteAdapter::decrypt() {
  auto& secret_ctx = std::get<1>(m_concrete_ctx);
  if (secret_ctx == nullptr) {
    throw std::logic_error("decrypt secret_ctx null !");
  }
  m_output_de = secret_ctx->decrypt(*m_concrete_output_en);
  return 0;
}

int ConcreteAdapter::execute(Dag& dag) {
  auto& public_ctx = std::get<0>(m_concrete_ctx);
  if (public_ctx == nullptr) {
    throw std::logic_error("execute public_ctx null !");
  }
  m_concrete_output_en = public_ctx->execute(dag, *m_concrete_valution);
  return 0;
}

int ConcreteAdapter::mergeInput(
    std::unique_ptr<ConcreteValuation>& p_valuation) {
  if (m_concrete_valution == nullptr) {
    m_concrete_valution = std::move(p_valuation);
  } else {
    m_concrete_valution->addValues(p_valuation);
  }
  return 0;
}

}  // namespace iyfc