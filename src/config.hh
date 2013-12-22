
#ifndef __CONFIG_HH_
#define __CONFIG_HH_

#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <sstream>

#include "bits/config.hh"
#include "bits/debug.hh"


////////////////////////////////////////////////////////////////////////////////////////// 
#define CFG config::instance()

using _Iter = std::string::iterator;

class config_io_error : public std::runtime_error {
public:
    config_io_error(std::string fname)
        : std::runtime_error(fname)
    {}
};

class config_key_error : public std::runtime_error {
public:
    config_key_error(std::string key)
        : std::runtime_error(key)
    {}
};

class config_parse_exception : public std::runtime_error {
public:
    config_parse_exception(std::string fname)
        : std::runtime_error(fname)
    {}
    
    config_parse_exception(std::string key, _Iter iter, size_t lsize = 12
                         , size_t rsize = 12)
        : std::runtime_error(parse_error(key, iter, lsize, rsize))
    {}

private:
    static std::string 
    parse_error(std::string key, _Iter iter, size_t lsize, size_t rsize) {
        std::stringstream ss;

        ss << "ERROR [" << key << "]" << std::endl;
        ss << "  --> '" << std::string((iter - lsize), (iter + rsize)) << "'";
        return ss.str();
    }
};


//////////////////////////////////////////////////////////////////////////////////////////
// KWARG ACCESS
//////////////////////////////////////////////////////////////////////////////////////////

class kwarg {
public:
    enum TYPE { FLOATING, INTEGRAL, STRING, SECTION, UNDEFINED, VECTOR };

    kwarg(const std::string& name, TYPE type)
        : _M_name(name), _M_type(type)
    {}

    std::string
    name() const { return _M_name; }
    
    TYPE 
    type() const { return _M_type; }

    kwarg& operator=(const kwarg&) = delete;
    kwarg(const kwarg&) = delete;
    kwarg(kwarg&&) = delete;

private:
    const std::string _M_name;
    const TYPE _M_type;
};

class kwarg_const : public kwarg {
public:
    ///{@
    kwarg_const(int64_t data, std::string name)
        : kwarg(name, kwarg::INTEGRAL)
    { _M_data.integral = data; }

    kwarg_const(double data, std::string name)
        : kwarg(name, kwarg::FLOATING)
    { _M_data.floating = data; }

    kwarg_const(std::string data, std::string name)
        : kwarg(name, kwarg::STRING)
    { _M_data.str = data; }
    ///@}
    
    ///{@
    template <typename _Tp>
    typename std::enable_if<std::is_integral<_Tp>::value, _Tp>::type
    as() const {
        return static_cast<_Tp>(_M_data.integral);
    }

    template <typename _Tp>
    typename std::enable_if<std::is_floating_point<_Tp>::value, _Tp>::type
    as() const {
        return static_cast<_Tp>(_M_data.floating);
    }

    template <typename _Tp>
    typename std::enable_if<std::is_same<std::string, _Tp>::value, _Tp>::type
    as() const {
        return _M_data.str;
    }
    ///@}

private:
    /**
     */
    union __kwarg_const_union {
        int64_t     integral;
        double      floating;
        std::string str;

        __kwarg_const_union()
        { new (&str) std::string(); }

        ~__kwarg_const_union() 
        { (&str)->~basic_string(); };
    } _M_data;
};


//////////////////////////////////////////////////////////////////////////////////////////
// CONFIG SECTIONS
//////////////////////////////////////////////////////////////////////////////////////////

class config_section : public kwarg {

    template <typename _K>
    struct is_not_customized {
        static constexpr bool value = std::is_pointer<_K>::value;
    };

public:
    ///{@
    config_section& operator=(const config_section&) = delete;
    config_section(const config_section&) = delete;
    config_section(config_section&&) = delete;
    ///@}
    
    config_section* section(const std::string& name);
    
    ///{@
    /**
     */
    template <typename _Tp>
    _Tp
    get(const std::string& key) {
        return static_cast<kwarg_const*>(_M_get_kwarg(key))->as<_Tp>();
    }
    
    /**
     */
    template <typename _Tp, _Tp deflt>
    _Tp
    get_default(const std::string& key) {
        auto it = _M_kwargs.find(key);

        if (it == _M_kwargs.end())
            return deflt;
        else 
            return static_cast<kwarg_const*>(_M_get_kwarg(key))->as<_Tp>();
    }
    ///@}
    
    bool has_kwarg(const std::string& key) const;
    bool has_section(const std::string& key) const;

    void dump(int depth = 0);

protected:
    friend class config_parser;
    config_section(const std::string& name);
    
    void _M_set_kwarg(kwarg* val);
    kwarg* _M_get_kwarg(const std::string& key) const;
   
    void _M_parse_file(const std::string& file_path, parse_trie<std::string>* regs);
    void _M_parse_iterator(_Iter& iter, parse_trie<std::string>* regs);
    void _M_parse_macro(_Iter& iter, parse_trie<std::string>* regs);
    kwarg* _M_parse_kwarg(std::string key, _Iter& iter, parse_trie<std::string>* regs);
    
    void _M_parse_define(_Iter& iter, parse_trie<std::string>* regs);
    void _M_parse_export(_Iter& iter, parse_trie<std::string>* regs);
    void _M_parse_include(_Iter& iter, parse_trie<std::string>* regs);

private:
    std::map<std::string, kwarg*> _M_kwargs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CONFIG ROOT OBJECT
//////////////////////////////////////////////////////////////////////////////////////////

class config : public config_section {

    static config* _S_instance;

public:
    static config* initialize(const std::string& file_path);
    static config* instance();
    
    /**
     * Tests down a hierarchy against a casting type.  This function should be used to
     * ensure types are being parsed correctly.
     *
     * eg:
     *   config::instance()->section("s0")->section("s1")->get<float>("key");
     *   config::instance()->assert_type("s0.s1.key", kwarg::FLOATING);
     */
    bool assert_type(const std::string& key, kwarg::TYPE type) const;

private:
    config(const std::string& file_path);
    
    parse_trie<std::string> _M_macro_regs;
};

#endif //__CONFIG_HH_