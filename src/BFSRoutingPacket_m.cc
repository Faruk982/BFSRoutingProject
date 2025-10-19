//
// Generated file, do not edit! Created by opp_msgtool 6.2 from BFSRoutingPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "BFSRoutingPacket_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

Register_Class(BFSRoutingPacket)

BFSRoutingPacket::BFSRoutingPacket(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

BFSRoutingPacket::BFSRoutingPacket(const BFSRoutingPacket& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

BFSRoutingPacket::~BFSRoutingPacket()
{
}

BFSRoutingPacket& BFSRoutingPacket::operator=(const BFSRoutingPacket& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void BFSRoutingPacket::copy(const BFSRoutingPacket& other)
{
    this->type = other.type;
    this->sourceAddress = other.sourceAddress;
    this->destinationAddress = other.destinationAddress;
    this->hopCount = other.hopCount;
    this->requestId = other.requestId;
    this->gCost = other.gCost;
    this->hCost = other.hCost;
    this->fCost = other.fCost;
    this->pathLength = other.pathLength;
    for (size_t i = 0; i < 10; i++) {
        this->path[i] = other.path[i];
    }
    this->cumulativeDelay = other.cumulativeDelay;
    this->timestamp = other.timestamp;
}

void BFSRoutingPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->type);
    doParsimPacking(b,this->sourceAddress);
    doParsimPacking(b,this->destinationAddress);
    doParsimPacking(b,this->hopCount);
    doParsimPacking(b,this->requestId);
    doParsimPacking(b,this->gCost);
    doParsimPacking(b,this->hCost);
    doParsimPacking(b,this->fCost);
    doParsimPacking(b,this->pathLength);
    doParsimArrayPacking(b,this->path,10);
    doParsimPacking(b,this->cumulativeDelay);
    doParsimPacking(b,this->timestamp);
}

void BFSRoutingPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->type);
    doParsimUnpacking(b,this->sourceAddress);
    doParsimUnpacking(b,this->destinationAddress);
    doParsimUnpacking(b,this->hopCount);
    doParsimUnpacking(b,this->requestId);
    doParsimUnpacking(b,this->gCost);
    doParsimUnpacking(b,this->hCost);
    doParsimUnpacking(b,this->fCost);
    doParsimUnpacking(b,this->pathLength);
    doParsimArrayUnpacking(b,this->path,10);
    doParsimUnpacking(b,this->cumulativeDelay);
    doParsimUnpacking(b,this->timestamp);
}

const char * BFSRoutingPacket::getType() const
{
    return this->type.c_str();
}

void BFSRoutingPacket::setType(const char * type)
{
    this->type = type;
}

int BFSRoutingPacket::getSourceAddress() const
{
    return this->sourceAddress;
}

void BFSRoutingPacket::setSourceAddress(int sourceAddress)
{
    this->sourceAddress = sourceAddress;
}

int BFSRoutingPacket::getDestinationAddress() const
{
    return this->destinationAddress;
}

void BFSRoutingPacket::setDestinationAddress(int destinationAddress)
{
    this->destinationAddress = destinationAddress;
}

int BFSRoutingPacket::getHopCount() const
{
    return this->hopCount;
}

void BFSRoutingPacket::setHopCount(int hopCount)
{
    this->hopCount = hopCount;
}

int BFSRoutingPacket::getRequestId() const
{
    return this->requestId;
}

void BFSRoutingPacket::setRequestId(int requestId)
{
    this->requestId = requestId;
}

double BFSRoutingPacket::getGCost() const
{
    return this->gCost;
}

void BFSRoutingPacket::setGCost(double gCost)
{
    this->gCost = gCost;
}

double BFSRoutingPacket::getHCost() const
{
    return this->hCost;
}

void BFSRoutingPacket::setHCost(double hCost)
{
    this->hCost = hCost;
}

double BFSRoutingPacket::getFCost() const
{
    return this->fCost;
}

void BFSRoutingPacket::setFCost(double fCost)
{
    this->fCost = fCost;
}

int BFSRoutingPacket::getPathLength() const
{
    return this->pathLength;
}

void BFSRoutingPacket::setPathLength(int pathLength)
{
    this->pathLength = pathLength;
}

size_t BFSRoutingPacket::getPathArraySize() const
{
    return 10;
}

int BFSRoutingPacket::getPath(size_t k) const
{
    if (k >= 10) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)10, (unsigned long)k);
    return this->path[k];
}

void BFSRoutingPacket::setPath(size_t k, int path)
{
    if (k >= 10) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)10, (unsigned long)k);
    this->path[k] = path;
}

double BFSRoutingPacket::getCumulativeDelay() const
{
    return this->cumulativeDelay;
}

void BFSRoutingPacket::setCumulativeDelay(double cumulativeDelay)
{
    this->cumulativeDelay = cumulativeDelay;
}

omnetpp::simtime_t BFSRoutingPacket::getTimestamp() const
{
    return this->timestamp;
}

void BFSRoutingPacket::setTimestamp(omnetpp::simtime_t timestamp)
{
    this->timestamp = timestamp;
}

class BFSRoutingPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_type,
        FIELD_sourceAddress,
        FIELD_destinationAddress,
        FIELD_hopCount,
        FIELD_requestId,
        FIELD_gCost,
        FIELD_hCost,
        FIELD_fCost,
        FIELD_pathLength,
        FIELD_path,
        FIELD_cumulativeDelay,
        FIELD_timestamp,
    };
  public:
    BFSRoutingPacketDescriptor();
    virtual ~BFSRoutingPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(BFSRoutingPacketDescriptor)

BFSRoutingPacketDescriptor::BFSRoutingPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(BFSRoutingPacket)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

BFSRoutingPacketDescriptor::~BFSRoutingPacketDescriptor()
{
    delete[] propertyNames;
}

bool BFSRoutingPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<BFSRoutingPacket *>(obj)!=nullptr;
}

const char **BFSRoutingPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *BFSRoutingPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int BFSRoutingPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 12+base->getFieldCount() : 12;
}

unsigned int BFSRoutingPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_type
        FD_ISEDITABLE,    // FIELD_sourceAddress
        FD_ISEDITABLE,    // FIELD_destinationAddress
        FD_ISEDITABLE,    // FIELD_hopCount
        FD_ISEDITABLE,    // FIELD_requestId
        FD_ISEDITABLE,    // FIELD_gCost
        FD_ISEDITABLE,    // FIELD_hCost
        FD_ISEDITABLE,    // FIELD_fCost
        FD_ISEDITABLE,    // FIELD_pathLength
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_path
        FD_ISEDITABLE,    // FIELD_cumulativeDelay
        FD_ISEDITABLE,    // FIELD_timestamp
    };
    return (field >= 0 && field < 12) ? fieldTypeFlags[field] : 0;
}

const char *BFSRoutingPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "type",
        "sourceAddress",
        "destinationAddress",
        "hopCount",
        "requestId",
        "gCost",
        "hCost",
        "fCost",
        "pathLength",
        "path",
        "cumulativeDelay",
        "timestamp",
    };
    return (field >= 0 && field < 12) ? fieldNames[field] : nullptr;
}

int BFSRoutingPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "type") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "sourceAddress") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "destinationAddress") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "hopCount") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "requestId") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "gCost") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "hCost") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "fCost") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "pathLength") == 0) return baseIndex + 8;
    if (strcmp(fieldName, "path") == 0) return baseIndex + 9;
    if (strcmp(fieldName, "cumulativeDelay") == 0) return baseIndex + 10;
    if (strcmp(fieldName, "timestamp") == 0) return baseIndex + 11;
    return base ? base->findField(fieldName) : -1;
}

const char *BFSRoutingPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_type
        "int",    // FIELD_sourceAddress
        "int",    // FIELD_destinationAddress
        "int",    // FIELD_hopCount
        "int",    // FIELD_requestId
        "double",    // FIELD_gCost
        "double",    // FIELD_hCost
        "double",    // FIELD_fCost
        "int",    // FIELD_pathLength
        "int",    // FIELD_path
        "double",    // FIELD_cumulativeDelay
        "omnetpp::simtime_t",    // FIELD_timestamp
    };
    return (field >= 0 && field < 12) ? fieldTypeStrings[field] : nullptr;
}

const char **BFSRoutingPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *BFSRoutingPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int BFSRoutingPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        case FIELD_path: return 10;
        default: return 0;
    }
}

void BFSRoutingPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'BFSRoutingPacket'", field);
    }
}

const char *BFSRoutingPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string BFSRoutingPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: return oppstring2string(pp->getType());
        case FIELD_sourceAddress: return long2string(pp->getSourceAddress());
        case FIELD_destinationAddress: return long2string(pp->getDestinationAddress());
        case FIELD_hopCount: return long2string(pp->getHopCount());
        case FIELD_requestId: return long2string(pp->getRequestId());
        case FIELD_gCost: return double2string(pp->getGCost());
        case FIELD_hCost: return double2string(pp->getHCost());
        case FIELD_fCost: return double2string(pp->getFCost());
        case FIELD_pathLength: return long2string(pp->getPathLength());
        case FIELD_path: return long2string(pp->getPath(i));
        case FIELD_cumulativeDelay: return double2string(pp->getCumulativeDelay());
        case FIELD_timestamp: return simtime2string(pp->getTimestamp());
        default: return "";
    }
}

void BFSRoutingPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: pp->setType((value)); break;
        case FIELD_sourceAddress: pp->setSourceAddress(string2long(value)); break;
        case FIELD_destinationAddress: pp->setDestinationAddress(string2long(value)); break;
        case FIELD_hopCount: pp->setHopCount(string2long(value)); break;
        case FIELD_requestId: pp->setRequestId(string2long(value)); break;
        case FIELD_gCost: pp->setGCost(string2double(value)); break;
        case FIELD_hCost: pp->setHCost(string2double(value)); break;
        case FIELD_fCost: pp->setFCost(string2double(value)); break;
        case FIELD_pathLength: pp->setPathLength(string2long(value)); break;
        case FIELD_path: pp->setPath(i,string2long(value)); break;
        case FIELD_cumulativeDelay: pp->setCumulativeDelay(string2double(value)); break;
        case FIELD_timestamp: pp->setTimestamp(string2simtime(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'BFSRoutingPacket'", field);
    }
}

omnetpp::cValue BFSRoutingPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: return pp->getType();
        case FIELD_sourceAddress: return pp->getSourceAddress();
        case FIELD_destinationAddress: return pp->getDestinationAddress();
        case FIELD_hopCount: return pp->getHopCount();
        case FIELD_requestId: return pp->getRequestId();
        case FIELD_gCost: return pp->getGCost();
        case FIELD_hCost: return pp->getHCost();
        case FIELD_fCost: return pp->getFCost();
        case FIELD_pathLength: return pp->getPathLength();
        case FIELD_path: return pp->getPath(i);
        case FIELD_cumulativeDelay: return pp->getCumulativeDelay();
        case FIELD_timestamp: return pp->getTimestamp().dbl();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'BFSRoutingPacket' as cValue -- field index out of range?", field);
    }
}

void BFSRoutingPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        case FIELD_type: pp->setType(value.stringValue()); break;
        case FIELD_sourceAddress: pp->setSourceAddress(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_destinationAddress: pp->setDestinationAddress(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_hopCount: pp->setHopCount(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_requestId: pp->setRequestId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_gCost: pp->setGCost(value.doubleValue()); break;
        case FIELD_hCost: pp->setHCost(value.doubleValue()); break;
        case FIELD_fCost: pp->setFCost(value.doubleValue()); break;
        case FIELD_pathLength: pp->setPathLength(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_path: pp->setPath(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_cumulativeDelay: pp->setCumulativeDelay(value.doubleValue()); break;
        case FIELD_timestamp: pp->setTimestamp(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'BFSRoutingPacket'", field);
    }
}

const char *BFSRoutingPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr BFSRoutingPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void BFSRoutingPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    BFSRoutingPacket *pp = omnetpp::fromAnyPtr<BFSRoutingPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'BFSRoutingPacket'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp

