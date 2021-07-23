/*****************************************************************/
/***
 *** Copyright (c) 2021 by SEP AG. All rights reserved.
 ***
 *** Reproduction is prohibited without written permission of
 *** SEP AG.
 ***
 ***/
 /*****************************************************************/
#ifndef SHARED_CONTEINERS_H
#define SHARED_CONTEINERS_H

#include <utility>
#include <mutex>
#include <iterator>
#include <set>

namespace NSharedConteiners
{
   template <typename T>
   class set
   {
      mutable std::mutex m_mutex;
      std::set<T> m_conteiner;

      set& operator=(const set&);
      set(const set& other);

   public:
      set() {}
      ~set() {}

      template <class... _Valty>
      std::pair<typename std::set<T>::iterator, bool>  emplace(_Valty&&... _Val)
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_conteiner.emplace(std::forward<_Valty>(_Val)...);
      }

      size_t size() const
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_conteiner.size();
      }
   };

}  // namespace NConteiner

#endif // SHARED_CONTEINERS_H