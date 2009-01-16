/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/utility.h"

IMP_BEGIN_NAMESPACE

std::string to_upper_case(const std::string & s)
{
  std::string upperS = s;
  for (unsigned int i = 0; i < upperS.size(); i++) {
    upperS[i] = std::toupper(upperS[i]);
  }
  return upperS;
}

std::string to_lower_case(const std::string & s)
{
  std::string lowerS = s;
  for (unsigned int i = 0; i < lowerS.size(); i++) {
    lowerS[i] = std::tolower(lowerS[i]);
  }
  return lowerS;
}

void string_split(const std::string & str, std::string delim,
                  std::vector<std::string> &results)
{
  int cutAt;
  std::string tmp = str;
  while ((cutAt = tmp.find_first_of(delim)) != (int)tmp.npos) {
    if (cutAt > 0) {
      results.push_back(tmp.substr(0, cutAt));
    }
    tmp = tmp.substr(cutAt + 1);
  }

  if (tmp.length() > 0) {
    results.push_back(tmp);
  }
}

bool is_blank_line(const std::string & line)
{
  for (unsigned int i = 0; i < line.length(); i++) {
    if (line[i] != ' ' && line[i] != '\t') {
      return false;
    }
  }
  return true;
}

IMP_END_NAMESPACE
