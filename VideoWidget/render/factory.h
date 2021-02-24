#ifndef FACTORY_H
#define FACTORY_H

#include <map>
#include <functional>
template <class IdentifierType, class ProductType>
class DefaultFactoryError
{
public:
    class Exception : public std::exception
    {
    public:
        Exception(const IdentifierType& unkownId)
            :unknownId_(unkownId){}

        const char* what() const noexcept override
        {
            return "Unknown object type passed to Factory";
        }

        const IdentifierType GetId()
        {
            return unknownId_;
        }

    private:
        IdentifierType unknownId_;
    };

protected:
    ProductType* OnUnknownType(const IdentifierType& id)
    {
        throw Exception(id);
    }
};

template
<
    class AbstractProduct,
    class IdentifierType,
    class ProductCreator = std::function<AbstractProduct*(void)>,
    template<typename, class>
        class FactoryErrorPolicy = DefaultFactoryError
>
class Factory
    : public FactoryErrorPolicy<IdentifierType, AbstractProduct>
{
public:
    bool Register(const IdentifierType& id, ProductCreator functor)
    {
        return associations_.insert(std::make_pair(id, functor)).second;
    }

    bool Unregister(const IdentifierType& id)
    {
        return associations_.erase(id);
    }

    AbstractProduct* CreateObject(const IdentifierType& id)
    {
        typename std::map<IdentifierType, ProductCreator>::const_iterator i = associations_.find(id);
        if(i != associations_.end())
        {
            return (i->second)();
        }

        return this->OnUnknownType(id);
    }

private:
    std::map<IdentifierType, ProductCreator> associations_;
};

#endif // FACTORY_H
