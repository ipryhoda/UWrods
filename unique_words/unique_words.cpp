// unique_words.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <boost/iostreams/device/mapped_file.hpp>

#include <functional>
#include <iostream>
#include <iterator>
#include <vector>
#include <thread>
#include <fstream>
#include <chrono>
#include <set>

#include "shared_conteiners.h"

//#define _EXPEREMENTIAL_BASIC_STRING_
#ifdef _EXPEREMENTIAL_BASIC_STRING_
namespace NString
{
   template<typename char_t>
   class _basic_string_proxy
   {
      _basic_string_proxy( _basic_string_proxy&&) = delete;
      _basic_string_proxy(const _basic_string_proxy&)  = delete;
      _basic_string_proxy& operator=(const _basic_string_proxy&) = delete;
   public:
      _basic_string_proxy(const char_t* buffer, size_t count) : _buffer(buffer), _count(count) {}
      ~_basic_string_proxy() {}

      bool operator<(const _basic_string_proxy& other) const
      {
         size_t idx = 0;
         while (idx < _count && idx < other._count)
         {
            if (_buffer[idx] != other._buffer[idx])
            {
               break;
            }

            idx++;
         }

         return *((const unsigned char*)&_buffer[idx]) - *((const unsigned char*)&other._buffer[idx]) < 0;
      }

      friend std::basic_ostream<char_t>& operator<<(std::basic_ostream<char_t>& os, const _basic_string_proxy<char_t>& obj)
      {
         os.write(obj._buffer, obj._count);

         return os;
      }

   private:
      const char_t* _buffer;
      size_t _count;
   };

#if 0
   template <>
   struct equal_to<_basic_string_proxy <char> >
   {
      size_t operator()(const _basic_string_proxy <char>& obj1, const _basic_string_proxy <char>& obj2) const { return obj1 == obj2;  }
   };

   template <>
   struct hash<_basic_string_proxy<char>>
   {
      size_t operator()(const _basic_string_proxy<char>& obj) const { return std::hash<std::basic_string<char>>()(obj.data()); }
   };
#endif
}  // namespace std
#endif

#define _MAPPED_FILE_APPROACH_
#ifdef _MAPPED_FILE_APPROACH_

template<typename char_t, typename conteiner_t>
void process_portion_data(const char_t* begin, const char_t* end, conteiner_t& unique_words)

{
   auto skip_unneeded_characters = [&]() {
      while (begin && begin != end)
      {
         if (*begin == ' ' || *begin == '\r' || *begin == '\0' || *begin == '\n') begin++;
         else break;
      }
   };

   skip_unneeded_characters();

   while (begin && begin != end)
   {
      const auto word = begin;

      while (begin && begin != end && *begin != ' ' && *begin != '\r' && *begin != '\n') begin++;

      auto result = unique_words.emplace(word, begin - word);
      if (!result.second)
      {
         // skipped
      }

      skip_unneeded_characters();
   }
}

#define _ASYNCH_FEATURE_IMPLEMENTAION

template <typename char_t>
size_t count_unique_words(const std::basic_string<char_t>& sFile) {
   boost::iostreams::mapped_file mfile(sFile, boost::iostreams::mapped_file::readonly);
   auto data = mfile.const_data(), end = data + mfile.size();

#ifdef _ASYNCH_FEATURE_IMPLEMENTAION
   const auto processor_count = std::thread::hardware_concurrency();   
   auto region = (end - data) / processor_count;
   std::vector<std::thread> workers;
  
   std::vector<std::set< std::basic_string<char_t>> > vec_unique_words(processor_count);

   for (unsigned i = 0; i < processor_count; ++i)
   {
      size_t uOffset = 0;

      if (i != processor_count - 1)
      {
         auto buffer = data + region;

         while (buffer && buffer != end && *buffer != ' ' && *buffer != '\r' && *buffer != '\n') { buffer++; uOffset++; }

         workers.push_back(std::thread(process_portion_data<char, std::set< std::basic_string<char_t>>>, data, data + region + uOffset, std::ref(vec_unique_words[i])));
      }
      else
      {
         workers.push_back(std::thread(process_portion_data<char, std::set< std::basic_string<char_t>>>, data, end, std::ref(vec_unique_words[i])));
      }

      data += region + uOffset;

   }
      
   for (auto&& worker : workers)
   {
      if (worker.joinable())
      {
         worker.join();
      }
   }
#else
   process_portion_data(data, end, vec_unique_words[0]);
#endif

   size_t uCount = 0;
   for (auto&& unique_words : vec_unique_words)
   {
      uCount += unique_words.size();
   }

   return uCount;
}

#else
template <typename char_t>
size_t count_unique_words(const std::basic_string<char_t>& sFile) {
#ifndef _EXPEREMENTIAL_BASIC_STRING_
   std::set< NString::_basic_string_proxy<char_t>> usWords;
#else
   std::set< std::basic_string<char_t>> usWords;
#endif

   std::ifstream ifs(sFile);

   std::vector<std::basic_string<char_t> > vecWords(std::istream_iterator<std::basic_string<char_t>>(ifs), {});
   for (auto word : vecWords)
   {
      usWords.insert(word);
   }

   return usWords.size();
}

#endif

int main(int argc, const char** argv)
{    
   if (argc > 1)
   {
      std::basic_ifstream<char> ifile(argv[1], std::ios::in);

      if (ifile.is_open())
      {
         auto start = std::chrono::high_resolution_clock::now();

         try
         {
            std::cout << "Result: " << count_unique_words<char>(argv[1]) << std::endl;            

         }
         catch (const std::exception& e)
         {
            std::cerr << "ERROR: " << e.what() << std::endl;
         }           

         auto stop = std::chrono::high_resolution_clock::now();
         auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

         std::cout << "Time taken by function: "
            << duration.count() << " microseconds" << std::endl;
      }
      else
      {
         std::cerr << "No file is specified as input" << std::endl;
      }
   }
   else
   {
      std::cerr << "No file is specified as input" << std::endl;
   }
}

