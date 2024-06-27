#pragma once

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

#include "revng/Model/Type.h"

/* TUPLE-TREE-YAML
name: ArrayType
type: struct
inherits: Type
fields:
  - name: ElementCount
    type: uint64_t
  - name: ElementType
    type: Type
    upcastable: true
TUPLE-TREE-YAML */

#include "revng/Model/Generated/Early/ArrayType.h"

class model::ArrayType : public model::generated::ArrayType {
public:
  static constexpr const auto AssociatedKind = TypeKind::ArrayType;

public:
  using generated::ArrayType::ArrayType;

  static UpcastableType make(UpcastableType &&ElementType,
                             uint64_t ElementCount) {
    return UpcastableType::make<ArrayType>(false,
                                           ElementCount,
                                           std::move(ElementType));
  }
};

#include "revng/Model/Generated/Late/ArrayType.h"
