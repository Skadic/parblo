
.. _program_listing_file_parblo_utils.hpp:

Program Listing for File utils.hpp
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_parblo_utils.hpp>` (``parblo/utils.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   #include <cstddef>
   
   namespace parblo::internal {
   
   static inline constexpr auto saturating_sub(size_t x, size_t y) -> size_t {
       size_t res = x - y;
       res &= -(res <= x);
       return res;
   }
   
   static inline constexpr auto bit_size(size_t v) -> size_t {
       return static_cast<size_t>(ceil(log2(static_cast<double>(v))));
   }
   } // namespace parblo::internal
