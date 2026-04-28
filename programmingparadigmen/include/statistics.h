#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include <iostream>
#include <map>
#include <variant>

namespace statistic {
	
/**
 * @brief Statistics class for managing statistical data of different types.
 * 
 * This class provides an interface to store, retrieve, update, and print 
 * various data types like int, float, double, and std::string using a key-based system.
 *
 * For performance some information is stored in variables.
 */
class Statistics {
public:
    long long no_of_shadowrays = 0;
    long long shadow_buffer_tests = 0;
    long long shadow_buffer_hits = 0;
    long long spatial_structure_no_intersection_tests = 0;
	long long spatial_structure_no_nearer_intersection_tests = 0;
    long long kd_tree_no_of_nodes_created = 0;
    float gamma_value = 2.2f; 
    bool hit_buffer_is_on = false;
	long long hit_buffer_hits = 0;
    bool shadow_buffer_is_on = false;
	
    /**
     * @brief Constructor for Statistics class.
     */
    Statistics() = default;

    /**
     * @brief Destructor for Statistics class.
     */
    ~Statistics() = default;

    /**
     * @brief Template method to access or create a value with the given key.
     * @tparam T The type of the value to access or create.
     * @param key The string key for the value.
     * @return Reference to the value associated with the key.
     */
    template<typename T>
    T& get(const std::string& key);

    /**
     * @brief Template method to set or update a value with the given key.
     * @tparam T The type of the value to set or update.
     * @param key The string key for the value.
     * @param value The value to set or update.
     */
    template<typename T>
    void set(const std::string& key, const T& value);

    /**
     * @brief Prints all stored data to the specified output stream.
     * @param os Output stream to print to, defaults to std::cout.
     */
    void printInfo(std::ostream& os = std::cout) const;

private:
    // Variant type to store different data types
    using DataType = std::variant<bool, int, long, long long, float, double, std::string>;
    
    // Map to store data with string keys
    std::map<std::string, DataType> m_data;
};

// Implementation of the template methods
template<typename T>
inline T& Statistics::get(const std::string& key) {
    if (m_data.find(key) == m_data.end()) {
        m_data[key] = T(); // Default construct the type T
    }
    return std::get<T>(m_data[key]);
}

template<typename T>
inline void Statistics::set(const std::string& key, const T& value) {
    m_data[key] = value;
}

/**
 * @brief Prints all stored data.
 */
inline void Statistics::printInfo(std::ostream& os) const {
    for (const auto& pair : m_data) {
        os << pair.first << ": ";
        std::visit([&os](auto&& arg) {
            os << arg << '\n';
        }, pair.second);
    }
	
	os << "kd_tree_no_of_nodes_created = " << kd_tree_no_of_nodes_created <<  '\n'
       << "spatial_structure_no_intersection_tests = " << spatial_structure_no_intersection_tests << '\n'
	   << "spatial_structure_no_nearer_intersection_tests = " << spatial_structure_no_nearer_intersection_tests << '\n'
       << "gamma_value = " << gamma_value << '\n'
       << "hit_buffer_is_on = " << hit_buffer_is_on << '\n'
	   << "hit_buffer_hits = " << hit_buffer_hits << '\n'
       << "no_of_shadowrays = " << no_of_shadowrays << '\n'
       << "shadow_buffer_is_on = " << shadow_buffer_is_on << '\n'
       << "shadow_buffer_tests = " << shadow_buffer_tests << '\n'
       << "shadow_buffer_hits = " << shadow_buffer_hits << '\n';
}

extern Statistics stat;
} // namespace
#endif

