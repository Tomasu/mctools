#include "tgetopt.h"
#include "NBT_Debug.h"
#include <stdexcept>
#include <sstream>
#include <cstring>

TGOParser::TGOParser(const std::vector<TGOOptionBase *> &optList) : optionList(optList)
{
	for(auto &opt: optList)
	{
		char so = findShortOpt(opt);
		if(so)
		{
			opt->setShortOpt(so);
			shortOptMap[so] = opt;
		}
		
		if(longOptMap.count(opt->name()))
		{
			std::ostringstream sstr; sstr << "duplicate option name " << opt->name();
			throw new std::runtime_error(sstr.str().c_str());
		}
		
		longOptMap[opt->name()] = opt;
	}
}


char TGOParser::findShortOpt(TGOOptionBase *opt)
{
	std::string optName = opt->name();
	
	for(size_t i = 0; i < optName.size(); i++)
	{
		char so = optName[i];
		
		if(!shortOptMap.count(so))
			return so;
	}
	
	return 0;
}

bool TGOParser::validate(const int &argc, const char **argv)
{
	bool escape_mode = false;
	for(int i = 1; i < argc; i++)
	{
		printf("arg: %s\n", argv[i]);
		int len = strlen(argv[i]);
		if(!escape_mode && len >= 2)
		{
			if(argv[i][0] == '-')
			{
				if(argv[i][1] == '-')
				{
					printf("long opt: %s\n", argv[i]);
					// long option, or -- separator
					if(len == 2) // -- separator
					{
						// all following command line parameters are passed in as is.
						// no parsing options
						escape_mode = true;
						continue;
					}
					else
					{
						char *eq = (char *)index(argv[i], '=');
						if(eq)
						{
							int name_len = eq - argv[i] - 2;
							int value_len = len - name_len;
							std::string optName(argv[i]+2, name_len);
							std::string optValue(eq+1, value_len);
							
							printf("opt: '%s'='%s'\n", optName.c_str(), optValue.c_str());
							
							if(!longOptMap.count(optName))
							{
								printf("unknown option %s.\n", optName.c_str());
								return false;
							}
							
							TGOOptionBase *opt = longOptMap[optName];
							if(opt->valueRequired())
							{
								if(!optValue.size())
								{
									printf("option %s requires a value.\n", optName.c_str());
									return false;
								}
								
								if(opt->validate(optValue))
									opt->setOptionValue(optValue);
								else
								{
									printf("option %s value is invalue.\n", argv[i]);
									return false;
								}
							}
							else
							{
								opt->setOptionValue("1");
							}
								
						}
						else
						{
							if(longOptMap.count(argv[i]+2))
							{
								TGOOptionBase *opt = longOptMap[argv[i]+2];
								if(opt->valueRequired())
								{
									if(i+1 >= argc)
									{
										printf("option %s requires a value.\n", argv[i]);
										return false;
									}
									
									const char *nextArg = argv[i+1];
									size_t nextArgLen = strlen(nextArg);
									const char *nextArgValue = 0;
									
									if(nextArgLen)
									{
										if(nextArg[0] == '-')
										{
											if(nextArgLen == 2 && nextArg[1] == '-')
											{
												// -- escape arg, pull in argv[i+2]
												if(i+2 >= argc)
												{
													printf("option %s requires a value.\n", argv[i]);
													return false;
												}
												
												nextArgValue = argv[i+2];
												i += 2;
											}
											else if(nextArgLen > 1)
											{
												printf("option %s requires a value.\n", argv[i]);
												return false;
											}
											else
											{
												// singular - (dash) signifies stdin
												nextArgValue = argv[i+1];
												i++;
											}
										}
										else
										{
											nextArgValue = argv[i+1];
											i++;
										}
									}
									else
									{
										printf("option %s requires a value.\n", argv[i]);
										return false;
									}
									
									if(opt->validate(nextArgValue))
										opt->setOptionValue(nextArgValue);
									else
										printf("option %s value is invalid.\n", argv[i]);
								}
								else
								{
									opt->setOptionValue("1");
								}
							}
							else
							{
								printf("unknown option %s\n", argv[i]);
								return false;
							}
						}
					}
				}
				else
				{
					// short option, or - stdin marker
					if(len == 1)
					{
						// stdin marker
						extra_parameters.push_back(argv[i]);
					}
					else
					{
						for(int j = 1; j < len; j++)
						{
							if(!shortOptMap.count(argv[i][j]))
							{
								printf("unknown option %c\n", argv[i][j]);
								return false;
							}
								
							if(shortOptMap[argv[i][j]]->valueRequired())
							{
								if(j <= len)
								{
									printf("option %c requires a value.\n", argv[i][j]);
									return false;
								}
								
								if(shortOptMap[argv[i][j]]->validate(argv[i+1]))
								{
									shortOptMap[argv[i][j]]->setOptionValue(argv[i+1]);
									printf("opt: '%c'='%s'\n", argv[i][j], argv[i+1]);
									i++;
								}
								else
								{
									printf("option %c value is invalid.\n", argv[i][j]);
									return false;
								}
							}
							else
							{
								shortOptMap[argv[i][j]]->setOptionValue("1");
							}
							
						}
					}
				}
			}
		}
		else
			extra_parameters.push_back(argv[i]);
	}
	
	return true;
}

TGOOptionBase *TGOParser::get(const std::string &optName)
{
	if(optName.size() == 1)
		return shortOptMap[optName[0]];
	
	return longOptMap[optName];
}

