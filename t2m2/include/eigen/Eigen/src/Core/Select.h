// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_SELECT_H
#define EIGEN_SELECT_H

/** \class Select
  *
  * \brief Expression of a coefficient wise version of the C++ ternary operator ?:
  *
  * \param ConditionMatrixType the type of the \em condition expression which must be a boolean matrix
  * \param ThenMatrixType the type of the \em then expression
  * \param ElseMatrixType the type of the \em else expression
  *
  * This class represents an expression of a coefficient wise version of the C++ ternary operator ?:.
  * It is the return type of DenseBase::select() and most of the time this is the only way it is used.
  *
  * \sa DenseBase::select(const DenseBase<ThenDerived>&, const DenseBase<ElseDerived>&) const
  */

template<typename ConditionMatrixType, typename ThenMatrixType, typename ElseMatrixType>
struct ei_traits<Select<ConditionMatrixType, ThenMatrixType, ElseMatrixType> >
 : ei_traits<ThenMatrixType>
{
  typedef typename ei_traits<ThenMatrixType>::Scalar Scalar;
  typedef Dense StorageKind;
  typedef typename ei_traits<ThenMatrixType>::XprKind XprKind;
  typedef typename ConditionMatrixType::Nested ConditionMatrixNested;
  typedef typename ThenMatrixType::Nested ThenMatrixNested;
  typedef typename ElseMatrixType::Nested ElseMatrixNested;
  enum {
    RowsAtCompileTime = ConditionMatrixType::RowsAtCompileTime,
    ColsAtCompileTime = ConditionMatrixType::ColsAtCompileTime,
    MaxRowsAtCompileTime = ConditionMatrixType::MaxRowsAtCompileTime,
    MaxColsAtCompileTime = ConditionMatrixType::MaxColsAtCompileTime,
    Flags = (unsigned int)ThenMatrixType::Flags & ElseMatrixType::Flags & HereditaryBits,
    CoeffReadCost = ei_traits<typename ei_cleantype<ConditionMatrixNested>::type>::CoeffReadCost
                  + EIGEN_SIZE_MAX(ei_traits<typename ei_cleantype<ThenMatrixNested>::type>::CoeffReadCost,
                                   ei_traits<typename ei_cleantype<ElseMatrixNested>::type>::CoeffReadCost)
  };
};

template<typename ConditionMatrixType, typename ThenMatrixType, typename ElseMatrixType>
class Select : ei_no_assignment_operator,
  public ei_dense_xpr_base< Select<ConditionMatrixType, ThenMatrixType, ElseMatrixType> >::type
{
  public:

    typedef typename ei_dense_xpr_base<Select>::type Base;
    EIGEN_DENSE_PUBLIC_INTERFACE(Select)

    Select(const ConditionMatrixType& conditionMatrix,
           const ThenMatrixType& thenMatrix,
           const ElseMatrixType& elseMatrix)
      : m_condition(conditionMatrix), m_then(thenMatrix), m_else(elseMatrix)
    {
      ei_assert(m_condition.rows() == m_then.rows() && m_condition.rows() == m_else.rows());
      ei_assert(m_condition.cols() == m_then.cols() && m_condition.cols() == m_else.cols());
    }

    Index rows() const { return m_condition.rows(); }
    Index cols() const { return m_condition.cols(); }

    const Scalar coeff(Index i, Index j) const
    {
      if (m_condition.coeff(i,j))
        return m_then.coeff(i,j);
      else
        return m_else.coeff(i,j);
    }

    const Scalar coeff(Index i) const
    {
      if (m_condition.coeff(i))
        return m_then.coeff(i);
      else
        return m_else.coeff(i);
    }

  protected:
    const typename ConditionMatrixType::Nested m_condition;
    const typename ThenMatrixType::Nested m_then;
    const typename ElseMatrixType::Nested m_else;
};


/** \returns a matrix where each coefficient (i,j) is equal to \a thenMatrix(i,j)
  * if \c *this(i,j), and \a elseMatrix(i,j) otherwise.
  *
  * Example: \include MatrixBase_select.cpp
  * Output: \verbinclude MatrixBase_select.out
  *
  * \sa class Select
  */
template<typename Derived>
template<typename ThenDerived,typename ElseDerived>
inline const Select<Derived,ThenDerived,ElseDerived>
DenseBase<Derived>::select(const DenseBase<ThenDerived>& thenMatrix,
                            const DenseBase<ElseDerived>& elseMatrix) const
{
  return Select<Derived,ThenDerived,ElseDerived>(derived(), thenMatrix.derived(), elseMatrix.derived());
}

/** Version of DenseBase::select(const DenseBase&, const DenseBase&) with
  * the \em else expression being a scalar value.
  *
  * \sa DenseBase::select(const DenseBase<ThenDerived>&, const DenseBase<ElseDerived>&) const, class Select
  */
template<typename Derived>
template<typename ThenDerived>
inline const Select<Derived,ThenDerived, typename ThenDerived::ConstantReturnType>
DenseBase<Derived>::select(const DenseBase<ThenDerived>& thenMatrix,
                            typename ThenDerived::Scalar elseScalar) const
{
  return Select<Derived,ThenDerived,typename ThenDerived::ConstantReturnType>(
    derived(), thenMatrix.derived(), ThenDerived::Constant(rows(),cols(),elseScalar));
}

/** Version of DenseBase::select(const DenseBase&, const DenseBase&) with
  * the \em then expression being a scalar value.
  *
  * \sa DenseBase::select(const DenseBase<ThenDerived>&, const DenseBase<ElseDerived>&) const, class Select
  */
template<typename Derived>
template<typename ElseDerived>
inline const Select<Derived, typename ElseDerived::ConstantReturnType, ElseDerived >
DenseBase<Derived>::select(typename ElseDerived::Scalar thenScalar,
                            const DenseBase<ElseDerived>& elseMatrix) const
{
  return Select<Derived,typename ElseDerived::ConstantReturnType,ElseDerived>(
    derived(), ElseDerived::Constant(rows(),cols(),thenScalar), elseMatrix.derived());
}

#endif // EIGEN_SELECT_H
