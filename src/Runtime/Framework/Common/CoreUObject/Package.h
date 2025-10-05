#pragma once
#include "Class.h"
#include "Common/Containers/Array.h"

namespace nilou {

/**
 * Wrapper for index into a ULnker's ImportMap or ExportMap.
 * Values greater than zero indicate that this is an index into the ExportMap.  The
 * actual array index will be (FPackageIndex - 1).
 *
 * Values less than zero indicate that this is an index into the ImportMap. The actual
 * array index will be (-FPackageIndex - 1)
 */
class FPackageIndex
{
	/**
	 * Values greater than zero indicate that this is an index into the ExportMap.  The
	 * actual array index will be (FPackageIndex - 1).
	 *
	 * Values less than zero indicate that this is an index into the ImportMap. The actual
	 * array index will be (-FPackageIndex - 1)
	 */
	int32 Index;

	/** Internal constructor, sets the index directly **/
	FORCEINLINE explicit FPackageIndex(int32 InIndex)
		: Index(InIndex)
	{

	}
public:
	/** Constructor, sets the value to null **/
	FORCEINLINE FPackageIndex()
		: Index(0)
	{

	}
	/** return true if this is an index into the import map **/
	FORCEINLINE bool IsImport() const
	{
		return Index < 0;
	}
	/** return true if this is an index into the export map **/
	FORCEINLINE bool IsExport() const
	{
		return Index > 0;
	}
	/** return true if this null (i.e. neither an import nor an export) **/
	FORCEINLINE bool IsNull() const
	{
		return Index == 0;
	}
	/** Check that this is an import and return the index into the import map **/
	FORCEINLINE int32 ToImport() const
	{
		Ncheck(IsImport());
		return -Index - 1;
	}
	/** Check that this is an export and return the index into the export map **/
	FORCEINLINE int32 ToExport() const
	{
		Ncheck(IsExport());
		return Index - 1;
	}
	/** Return the raw value, for debugging purposes**/
	FORCEINLINE int32 ForDebugging() const
	{
		return Index;
	}

	/** Create a FPackageIndex from an import index **/
	FORCEINLINE static FPackageIndex FromImport(int32 ImportIndex)
	{
		Ncheck(ImportIndex >= 0);
		return FPackageIndex(-ImportIndex - 1);
	}
	/** Create a FPackageIndex from an export index **/
	FORCEINLINE static FPackageIndex FromExport(int32 ExportIndex)
	{
		Ncheck(ExportIndex >= 0);
		return FPackageIndex(ExportIndex + 1);
	}

	/** Compare package indecies for equality **/
	FORCEINLINE bool operator==(const FPackageIndex& Other) const
	{
		return Index == Other.Index;
	}
	/** Compare package indecies for inequality **/
	FORCEINLINE bool operator!=(const FPackageIndex& Other) const
	{
		return Index != Other.Index;
	}

	/** Compare package indecies **/
	FORCEINLINE bool operator<(const FPackageIndex& Other) const
	{
		return Index < Other.Index;
	}
	FORCEINLINE bool operator>(const FPackageIndex& Other) const
	{
		return Index > Other.Index;
	}
	FORCEINLINE bool operator<=(const FPackageIndex& Other) const
	{
		return Index <= Other.Index;
	}
	FORCEINLINE bool operator>=(const FPackageIndex& Other) const
	{
		return Index >= Other.Index;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FPackageIndex& In)
	{
		return uint32(In.Index);
	}

	friend void Serialize(FArchive& Ar, FPackageIndex& Value);
};
void Serialize(FArchive& Ar, FPackageIndex& Value);

struct FObjectResource
{
    std::string ObjectName;

    FPackageIndex OuterIndex;
};
void Serialize(FArchive& Ar, FObjectResource& Value);

struct FObjectExport : public FObjectResource
{
    FPackageIndex ClassIndex;

    std::string ClassName;

    FPackageIndex ObjectIndex;

	NObject* Object = nullptr;
};
void Serialize(FArchive& Ar, FObjectExport& Value);

struct FObjectImport : public FObjectResource
{
	std::string PackageName;

	NObject* XObject = nullptr;
};
void Serialize(FArchive& Ar, FObjectImport& Value);

class NPackage : public NObject
{
private:
	template<typename T> 
	friend class TClassRegistry;
	static NClass* Z_StaticClass;
	friend struct FIntrinsicClassRegistry;
public:
	virtual NClass *GetClass() const override { return StaticClass(); }
	static NClass *StaticClass() { return Z_StaticClass; }

    virtual void Serialize(FArchive& Ar) override;

	static void SavePackage(NPackage* Package);

	bool IsDirty() const { return bDirty; }

	void SetDirtyFlag(bool bInDirty) { bDirty = bInDirty; }

private:
	bool bDirty = false;

};

NPackage GetTransientPackage();

}

