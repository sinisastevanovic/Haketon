#include "hkpch.h"
#include "ISerializationContext.h"

namespace Haketon
{
    rttr::variant IDeserializer::CreateDefaultVariant(rttr::type type)
    {
        if (type == rttr::type::get<bool>()) return false;
        if (type == rttr::type::get<char>()) return '\0';
        if (type == rttr::type::get<short>()) return (short)0;
        if (type == rttr::type::get<int>()) return 0;
        if (type == rttr::type::get<long>()) return (long)0;
        if (type == rttr::type::get<long long>()) return (long long)0;
        if (type == rttr::type::get<unsigned char>()) return (unsigned char)0;
        if (type == rttr::type::get<unsigned short>()) return (unsigned short)0;
        if (type == rttr::type::get<unsigned int>()) return (unsigned int)0;
        if (type == rttr::type::get<unsigned long>()) return (unsigned long)0;
        if (type == rttr::type::get<unsigned long long>()) return (unsigned long long)0;
        if (type == rttr::type::get<float>()) return 0.0f;
        if (type == rttr::type::get<double>()) return 0.0;
        if (type == rttr::type::get<long double>()) return (long double)0.0;
        if (type == rttr::type::get<std::string>()) return std::string("");

        if (type.is_enumeration()) {
            return (int)0; 
        }

        /*if (type == rttr::type::get<std::unique_ptr<void>>().get_raw_type().get_wrapped_type_ptr()) { // Generic unique_ptr check
            // For unique_ptr<T>, return a null unique_ptr<T>
            return rttr::variant(nullptr, type); // variant(nullptr, rttr::type::get<std::unique_ptr<SomeType>>())
        }
        if (type == rttr::type::get<std::shared_ptr<void>>().get_raw_type().get_wrapped_type_ptr()) { // Generic shared_ptr check
             // For shared_ptr<T>, return a null shared_ptr<T>
            return rttr::variant(std::shared_ptr<void>(), type); // variant(nullptr, rttr::type::get<std::shared_ptr<SomeType>>())
        }*/

        // 3. Handle classes/structs (requires a default constructor or a registered default constructor)
        if (type.is_class()) {
            // rttr::type::create() attempts to find a default constructor and call it.
            // It's the primary way to create instances of reflected classes.
            rttr::variant obj = type.create();
            if (obj.is_valid()) {
                return obj;
            } else {
                std::cerr << "RTTRExtensions Error: Could not create default instance for class '"
                          << type.get_name().to_string() << "'. No default constructor or creation policy specified." << std::endl;
            }
        }

        // 4. Handle containers (empty container)
        // RTTR's create() works for standard containers too
        if (type.is_sequential_container() || type.is_associative_container()) {
             rttr::variant container = type.create();
             if (container.is_valid()) {
                 return container;
             } else {
                 std::cerr << "RTTRExtensions Error: Could not create default instance for container type '"
                           << type.get_name().to_string() << "'." << std::endl;
             }
        }

        std::cerr << "RTTRExtensions Error: Failed to create default variant for type '"
                  << type.get_name().to_string() << "'." << std::endl;
        return rttr::variant(); // Return an invalid variant if creation failed
    }
}
