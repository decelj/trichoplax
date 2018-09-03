#pragma once

#include <string>
#include <sstream>
#include <cstdint>
#include <exception>
#include <unordered_set>
#include <vector>
#include <iostream>

class CLArgs
{
public:
    CLArgs();

    template<class T>
    void RegisterArg(const std::string& name, T* dest, const T& defaultValue);

    bool Parse(char** args, uint32_t count, std::vector<std::string>* extraArgs);
    void PrintUsage() const;

private:
    struct Arg
    {
        enum Type
        {
            INT,
            UNSIGNED,
            FLOAT,
            STRING,
            BOOL,
            INVALID
        };

        Arg(const std::string& _name, Type _type, void* _dest)
            : name(_name), type(_type), dest(_dest)
        {
        }

        Arg(const std::string& _name)
            : name(_name), type(INVALID), dest(nullptr)
        {
        }

        bool IsFlag() const { return type == BOOL; }
        bool operator==(const Arg& rhs) const { return name == rhs.name; }
        bool operator!=(const Arg& rhs) const { return !(*this == rhs); }

        bool Parse(const char* value) const;
        void Print() const;

        struct Hash
        {
            std::size_t operator()(const Arg& a) const noexcept
            {
                return std::hash<std::string>{}(a.name);
            }
        };

        template<class T>
        struct TypeToEnum
        {
        };

        const std::string name;
        const Type type;
        void* const dest;
    };
    using ArgSet = std::unordered_set<Arg, Arg::Hash>;

    template<class T>
    std::string StrConcat(const T& base)
    {
        return (std::ostringstream() << base).str();
    }

    template<class T, class... TArgs>
    std::string StrConcat(const T& base, TArgs&&... FArgs)
    {
        return std::string(base) + StrConcat(FArgs...);
    }

    ArgSet mArgs;
};


template<class T>
inline void CLArgs::RegisterArg(const std::string& name, T* dest, const T& defaultValue)
{
    Arg::Type typeEnum = Arg::TypeToEnum<T>::value;
    if (!mArgs.emplace(name, typeEnum, dest).second)
    {
        throw std::invalid_argument(StrConcat("Invalid argument ", name, ": already added"));
    }

    *dest = defaultValue;
}


template<>
struct CLArgs::Arg::TypeToEnum<int>
{
    static constexpr Type value = INT;
};

template<>
struct CLArgs::Arg::TypeToEnum<unsigned>
{
    static constexpr Type value = UNSIGNED;
};

template<>
struct CLArgs::Arg::TypeToEnum<float>
{
    static constexpr Type value = FLOAT;
};

template<>
struct CLArgs::Arg::TypeToEnum<bool>
{
    static constexpr Type value = BOOL;
};

template<>
struct CLArgs::Arg::TypeToEnum<std::string>
{
    static constexpr Type value = STRING;
};
