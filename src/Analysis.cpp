/* Copyright 2017 OpenABL Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "Analysis.hpp"
#include "AST.hpp"

namespace OpenABL {

uint32_t VarId::max_id;

// Concrete signature with any generic agent types replaced
FunctionSignature FunctionSignature::getConcreteSignature(
    const std::vector<Type> &argTypes) const {
  if (customGetConcreteSignature) {
    return customGetConcreteSignature(argTypes);
  }

  std::vector<Type> newParamTypes;
  Type newReturnType;
  Type agentType = Type::AGENT;
  for (size_t i = 0; i < paramTypes.size(); i++) {
    Type type = paramTypes[i];
    if (type.isGenericAgent()) {
      agentType = argTypes[i];
      newParamTypes.push_back(argTypes[i]);
    } else if (type.isGenericAgentArray()) {
      agentType = argTypes[i].getBaseType();
      newParamTypes.push_back({ Type::ARRAY, argTypes[i].getBaseType() });
    } else {
      newParamTypes.push_back(type);
    }
  }

  if (returnType.isGenericAgent()) {
    newReturnType = agentType;
  } else if (returnType.isGenericAgentArray()) {
    newReturnType = { Type::ARRAY, agentType };
  } else {
    newReturnType = returnType;
  }

  return { origName, name, newParamTypes, newReturnType, flags, decl };
}

}
