#include "common_tool.h"

CMyLogger *g_ECGLogger;// = CMyLogger::getInstance();

std::string time2str ( time_t time )
{
	tm* local;
	char time_str[ 128 ];

	local = localtime ( &time );
	memset ( time_str , 0 , 128 );
	strftime ( time_str , 64 , "%Y-%m-%d %H:%M:%S" , local );
	return time_str;
}

template<class T>
std::string num2str ( const T num )
{
	stringstream ss;
	std::string str;
	ss << num;
	ss >> str;
	return str;
}

std::string int2str ( const int &int_temp )
{
	std::string str;
	stringstream st;
	st << int_temp;
	st >> str;
	return str;
}

//string long2str ( const unsigned __int64 &double_temp )
//{
//	stringstream ss;
//	string str;
//	ss << double_temp;
//	ss >> str;
//	return str;
//}

std::string double2str ( const double &double_temp )
{
	stringstream ss;
	std::string str;
	ss << double_temp;
	ss >> str;
	return str;
}

std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;				//扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

// 文件名匹配
// 本字符串里保存着文件名, 参数传入匹配模板, 匹配成功返回 true, 否则返回 false
// 匹配时注意先自行转换大小写, 本匹配函数不做自动大小写转换
bool keyMatch(const std::string srcStr, const std::string & strPattern)
{
	if (srcStr.empty())
	{
		return false;
	}
	if (strPattern.empty())
	{
		return true;
	}
	//文件名做通配符匹配
	const char * ft = strPattern.c_str();
	const char * f = srcStr.c_str();
	const char * p1, *p2;

	size_t lt = (size_t)strPattern.length();
	size_t lf = (size_t)srcStr.length();

	//等下匹配要用到，需要记录用户有没有遇到 * 号
	//遇到 * 此变量值为 1
	//没遇到此变量值为 0
	int iMeetXing = 0;

	//等下匹配要用到，要记录用户遇到多少个? 号
	int iQuestionNumber = 0;

	// 当模板匹配完 * 号时, 有一个特殊情形, 即 A.1.DOC, 跟 *.DOC 匹配, .1.DOC 跟 .DOC 匹配不成功, 但是后面的 .DOC 应该匹配成功的, 这里就是这个标志
	bool bTempReset = false;


	if (lf == 0)
	{
		return false;
	}

	p1 = ft; //模板字符
	p2 = f; //文件名字符

	while (lf > 0 && lt > 0)
	{
		if (*p1 != *p2)
		{ //两者不等
			if (*p1 != '*'  &&  *p1 != '?')
			{
				if (iMeetXing > 0)
				{
					if (!bTempReset)
					{
						--p1;
						++lt;
						bTempReset = true;
					}
					else
					{
						++p2;
						--lf;
					}
					continue;
				}
				return false;
			}

			iMeetXing = 0;
			iQuestionNumber = 0;

			while (*p1 == '*' || *p1 == '?')
			{
				--lt;
				if (*p1 == '*')
				{
					iMeetXing = 1;
				}
				else
				{
					++iQuestionNumber;
				}
				++p1;

				if (lt == 0)
				{ //说明模板后面几个字符全部是通配符
					if (iQuestionNumber != 0 && iQuestionNumber != (int)lf)
					{
						return false;
					}
					return true;
				}
			}
			//处理遇到*号和?号的情况

			//首先是遇到了*号，直接匹配文件名下一个字符
			if (iMeetXing > 0)
			{
				//遇到*号的同时，也可能遇到 ? 号，每遇到一次 ? 号，要把文件名字符后移一个
				if ((int)lf <= iQuestionNumber)
				{
					return false;
				}
				lf -= iQuestionNumber;
				p2 += iQuestionNumber;

				do
				{
					if (*p2 == *p1)
					{
						++p2;
						++p1;

						--lt;
						--lf;
						bTempReset = false;
						break;
					}
					++p2;
					--lf;
				} while (lf > 0);
				continue;
			}
			else
			{ //没有遇到 * 号， 而是遇到了 ? 号
				if ((int)lf <= iQuestionNumber)
				{
					return false;
				}
				lf -= iQuestionNumber;
				p2 += iQuestionNumber;
				continue;
			}
		}//if( *p1 != *p2 )

		 //到这里表示相等
		bTempReset = false;
		++p1; //比较下一个字符
		++p2;

		--lt;
		--lf;
	}

	if (lt == 0 && lf == 0)
	{
		return true;
	}
	else if (lt > 0)
	{ //如果模板还有剩余，则剩余的内容必须全部是 * 号
		while (lt > 0)
		{
			if (*p1 != '*')
			{
				return false;
			}
			--lt;
			++p1;
		}
		return true;

	}
	return false;
}

std::string generate_uuid()
{
	char buf[UUID4_LEN];
	uuid4_generate(buf);
	
	string uuidStr = std::string(buf);
	uuidStr.erase(remove(uuidStr.begin(), uuidStr.end(), '-'), uuidStr.end());
	return uuidStr;
}
