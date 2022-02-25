// Copyright (c) 2020 The DeFi Foundation
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEFI_MASTERNODES_GOVVARIABLES_ATTRIBUTES_H
#define DEFI_MASTERNODES_GOVVARIABLES_ATTRIBUTES_H

#include <amount.h>
#include <masternodes/balances.h>
#include <masternodes/gv.h>

#include <variant>

enum VersionTypes : uint8_t {
    v0 = 0,
};

enum AttributeTypes : uint8_t {
    Live      = 'l',
    Param     = 'a',
    Token     = 't',
    Oracles   = 'o',
    Poolpairs = 'p',
};

enum ParamIDs : uint8_t  {
    DFIP2201  = 'a',
    Economy   = 'e',
};

enum OracleIDs : uint8_t  {
    Splits    = 'a',
};

enum EconomyKeys : uint8_t {
    PaybackDFITokens = 'a',
};

enum DFIP2201Keys : uint8_t  {
    Active    = 'a',
    Premium   = 'b',
    MinSwap   = 'c',
};

enum TokenKeys : uint8_t  {
    PaybackDFI       = 'a',
    PaybackDFIFeePCT = 'b',
    DexInFeePct      = 'c',
    DexOutFeePct     = 'd',
};

enum PoolKeys : uint8_t {
    TokenAFeePCT = 'a',
    TokenBFeePCT = 'b',
};

struct CDataStructureV0 {
    uint8_t type;
    uint32_t typeId;
    uint8_t key;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(type);
        READWRITE(typeId);
        READWRITE(key);
    }

    bool operator<(const CDataStructureV0& o) const {
        return std::tie(type, typeId, key) < std::tie(o.type, o.typeId, o.key);
    }
};

// for future use
struct CDataStructureV1 {
    uint32_t type;
    uint32_t typeId;
    uint32_t key;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(type);
        READWRITE(typeId);
        READWRITE(key);
    }

    bool operator<(const CDataStructureV1& o) const {
        return std::tie(type, typeId, key) < std::tie(o.type, o.typeId, o.key);
    }
};

struct OracleSplitValue {
    uint32_t tokenId;
    int32_t multiplier;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(tokenId);
        READWRITE(multiplier);
    }

    bool operator<(const OracleSplitValue& o) const {
        return std::tie(tokenId, multiplier) < std::tie(o.tokenId, o.multiplier);
    }
};

using CAttributeType = std::variant<CDataStructureV0, CDataStructureV1>;
using CAttributeValue = std::variant<bool, CAmount, CBalances, OracleSplitValue>;

class ATTRIBUTES : public GovVariable, public AutoRegistrator<GovVariable, ATTRIBUTES>
{
public:
    ~ATTRIBUTES() override = default;

    std::string GetName() const override {
        return TypeName();
    }

    Res Import(UniValue const &val) override;
    UniValue Export() const override;
    Res Validate(CCustomCSView const &mnview) const override;
    Res Apply(CCustomCSView &mnview, const uint32_t height) override;

    static constexpr char const * TypeName() { return "ATTRIBUTES"; }
    static GovVariable * Create() { return new ATTRIBUTES(); }

    template<typename T>
    T GetValue(const CAttributeType& key, T value) const {
        auto it = attributes.find(key);
        if (it != attributes.end()) {
            if (auto val = std::get_if<T>(&it->second)) {
                value = *val;
            }
        }
        return std::move(value);
    }

    ADD_OVERRIDE_VECTOR_SERIALIZE_METHODS
    ADD_OVERRIDE_SERIALIZE_METHODS(CDataStream)

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(attributes);
    }

    std::map<CAttributeType, CAttributeValue> attributes;

private:
    // Defined allowed arguments
    static const std::map<std::string, uint8_t>& allowedVersions();
    static const std::map<std::string, uint8_t>& allowedTypes();
    static const std::map<std::string, uint8_t>& allowedParamIDs();
    static const std::map<std::string, uint8_t>& allowedOracleIDs();
    static const std::map<uint8_t, std::map<std::string, uint8_t>>& allowedKeys();
    static const std::map<uint8_t, std::map<uint8_t,
            std::function<ResVal<CAttributeValue>(const std::string&)>>>& parseValue();

    // For formatting in export
    static const std::map<uint8_t, std::string>& displayVersions();
    static const std::map<uint8_t, std::string>& displayTypes();
    static const std::map<uint8_t, std::string>& displayParamsIDs();
    static const std::map<uint8_t, std::string>& displayOracleIDs();
    static const std::map<uint8_t, std::map<uint8_t, std::string>>& displayKeys();

    Res ProcessVariable(const std::string& key, const std::string& value,
                        const std::function<Res(const CAttributeType&, const CAttributeValue&)>& applyVariable) const;
};

#endif // DEFI_MASTERNODES_GOVVARIABLES_ATTRIBUTES_H
