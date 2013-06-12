#ifndef TGETOPT_H_GUARD
#define TGETOPT_H_GUARD

#include <string>
#include <vector>
#include <map>

class TGOOptionBase
{
	public:
		TGOOptionBase(const std::string &name, bool value_required) : optionName(name), valueRequired_(value_required), shortOpt(0) { }
		TGOOptionBase(const std::string &name) : optionName(name), valueRequired_(false), shortOpt(0) { }
		~TGOOptionBase() { }
		
		std::string name() { return optionName; }
		
		void setShortOpt(char so) { shortOpt = so; }
		
		std::string optionValue() { return optionValue_; }
		virtual void setOptionValue(const std::string &value) { optionValue_ = value; }
		
		virtual bool validate(const std::string &value) = 0;
		
		bool valueRequired() { return valueRequired_; }
	private:
		std::string optionName;
		bool valueRequired_;
		
		char shortOpt;
		
		std::string optionValue_;
};

template<typename T>
class TGOOption : public TGOOptionBase
{
	public:
		TGOOption(const std::string &name, bool value_required) : TGOOptionBase(name, value_required) { }
		TGOOption(const std::string &name) : TGOOptionBase(name) { }
		~TGOOption() { }
		
		virtual T value() = 0;
		virtual void setValue(T &value) = 0;
			
	private:
		
};

class TGOBoolOption : public TGOOption<bool>
{
	public:
		typedef bool value_type;
		
		TGOBoolOption(const std::string &name) : TGOOption(name), boolValue(false) { }
		virtual ~TGOBoolOption() { }
		
		virtual bool validate(const std::string &value) { return false; }
		
		virtual bool value()
		{
			return boolValue;
		}
		
		virtual void setValue(bool &value)
		{
			boolValue = value;
		}
		
		virtual void setOptionValue(const std::string &value)
		{
			if(!value.size() || value == "0" || value == "false")
				boolValue = false;
			else
				boolValue = true;
		}
		
	private:
		bool boolValue;
};

class TGOStringOption : public TGOOption<std::string>
{
	public:
		typedef std::string value_type;
		TGOStringOption(const std::string &name) : TGOOption(name, true) { }
		TGOStringOption(const std::string &name, const std::string &stringValue) : TGOOption(name, true), stringValue(stringValue) { }
		virtual ~TGOStringOption() { }
		
		virtual bool validate(const std::string &value)
		{
			return true; // all things are strings
		}
		
		virtual std::string value() { return stringValue; }
		
		virtual void setValue(std::string &value) { stringValue = value; }
		virtual void setOptionValue(const std::string &value)
		{
			stringValue = value;
		}
		
	private:
		std::string stringValue;
};

class TGOParser
{
	public:
		TGOParser(const std::vector<TGOOptionBase *> &optList);
		
		bool validate(const int &argc, const char **argv);
		
		TGOOptionBase *get(const std::string &optName);
		
		std::vector<std::string> extraParameters() { return extra_parameters; }
		
		template<typename T>
		typename T::value_type getValue(const std::string &optName)
		{
			T *opt = dynamic_cast<T *> (optName.size() == 1 ? shortOptMap[optName[0]] : longOptMap[optName]);
			if(!opt)
				return typename T::value_type();
			
			return opt->value();
		}
		
	private:
		std::map<char, TGOOptionBase *> shortOptMap;
		std::map<std::string, TGOOptionBase *> longOptMap;
		
		std::vector<TGOOptionBase *> optionList;
		std::vector<std::string> extra_parameters;
		
		char findShortOpt(TGOOptionBase *opt);
		
};

#endif /* TGETOPT_H_GUARD */
