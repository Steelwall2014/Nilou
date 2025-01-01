#pragma once
#include "IsArithmetic.h"

namespace nilou {

/**
 * Type traits for Arithmetic interval.
 */
template <typename ElementType> struct TIntervalTraits
{
	static_assert(TIsArithmetic<ElementType>::Value, "Incompatible TInterval element type.");

	static ElementType Max()
	{
		return std::numeric_limits<ElementType>::max();
	}

	static ElementType Lowest()
	{
		return std::numeric_limits<ElementType>::min();
	}
};

/**
 * Template for numeric interval
 */
template<typename ElementType> struct TInterval
{
	/** Holds the lower bound of the interval. */
	ElementType Min;
	
	/** Holds the upper bound of the interval. */
	ElementType Max;
		
public:

	/**
	 * Default constructor.
	 *
	 * The interval is invalid
	 */
	TInterval()
		: Min(TIntervalTraits<ElementType>::Max())
		, Max(TIntervalTraits<ElementType>::Lowest())
	{ }

    /**
	 * Creates and initializes a new interval with the specified lower and upper bounds.
	 *
	 * @param InMin The lower bound of the constructed interval.
	 * @param InMax The upper bound of the constructed interval.
	 */
	TInterval( ElementType InMin, ElementType InMax )
		: Min(InMin)
		, Max(InMax)
	{ }

public:

	/**
	 * Offset the interval by adding X.
	 *
	 * @param X The offset.
	 */
	void operator+= ( ElementType X )
	{
		if (IsValid())
		{
			Min += X;
			Max += X;
		}
	}

	/**
	 * Offset the interval by subtracting X.
	 *
	 * @param X The offset.
	 */
	void operator-= ( ElementType X )
	{
		if (IsValid())
		{
			Min -= X;
			Max -= X;
		}
	}

public:
	
	/**
	 * Computes the size of this interval.
	 *
	 * @return Interval size.
	 */
	ElementType Size() const
	{
		return (Max - Min);
	}

	/**
	 * Whether interval is valid (Min <= Max).
	 *
	 * @return false when interval is invalid, true otherwise
	 */
	bool IsValid() const
	{
		return (Min <= Max);
	}
	
	/**
	 * Checks whether this interval contains the specified element.
	 *
	 * @param Element The element to check.
	 * @return true if the range interval the element, false otherwise.
	 */
	bool Contains( const ElementType& Element ) const
	{
		return IsValid() && (Element >= Min && Element <= Max);
	}

	/**
	 * Expands this interval to both sides by the specified amount.
	 *
	 * @param ExpandAmount The amount to expand by.
	 */
	void Expand( ElementType ExpandAmount )
	{
		if (IsValid())
		{
			Min -= ExpandAmount;
			Max += ExpandAmount;
		}
	}

	/**
	 * Expands this interval if necessary to include the specified element.
	 *
	 * @param X The element to include.
	 */
	void Include( ElementType X )
	{
		if (!IsValid())
		{
			Min = X;
			Max = X;
		}
		else
		{
			if (X < Min)
			{
				Min = X;
			}

			if (X > Max)
			{
				Max = X;
			}
		}
	}

	/**
	 * Interval interpolation
	 *
	 * @param Alpha interpolation amount
	 * @return interpolation result
	 */
	ElementType Interpolate( float Alpha ) const
	{
		if (IsValid())
		{
			return Min + ElementType(Alpha*Size());
		}
		
		return ElementType();
	}

public:

	/**
	 * Calculates the intersection of two intervals.
	 *
	 * @param A The first interval.
	 * @param B The second interval.
	 * @return The intersection.
	 */
	friend TInterval Intersect( const TInterval& A, const TInterval& B )
	{
		if (A.IsValid() && B.IsValid())
		{
			return TInterval(std::max(A.Min, B.Min), std::max(A.Max, B.Max));
		}

		return TInterval();
	}

	/**
	 * Serializes the interval.
	 *
	 * @param Ar The archive to serialize into.
	 * @param Interval The interval to serialize.
	 * @return Reference to the Archive after serialization.
	 */
	friend class FArchive& operator<<( class FArchive& Ar, TInterval& Interval )
	{
		return Ar << Interval.Min << Interval.Max;
	}
	
	/**
	 * Gets the hash for the specified interval.
	 *
	 * @param Interval The Interval to get the hash for.
	 * @return Hash value.
	 */
	friend uint32 GetTypeHash(const TInterval& Interval)
	{
		return HashCombine(std::hash<ElementType>()(Interval.Min), std::hash<ElementType>()(Interval.Max));
	}
};

}

namespace std {

template<typename ElementType>
struct hash<nilou::TInterval<ElementType>>
{
	size_t operator()(const nilou::TInterval<ElementType> &_Keyval) const noexcept
	{
		return _Keyval.Hash;
	}
};

}