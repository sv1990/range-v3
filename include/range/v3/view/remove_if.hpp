/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_VIEW_REMOVE_IF_HPP
#define RANGES_V3_VIEW_REMOVE_IF_HPP

#include <utility>
#include <type_traits>
#include <range/v3/range_fwd.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/range_adaptor.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/utility/meta.hpp>
#include <range/v3/utility/pipeable.hpp>
#include <range/v3/utility/optional.hpp>
#include <range/v3/utility/invokable.hpp>
#include <range/v3/utility/functional.hpp>
#include <range/v3/utility/static_const.hpp>
#include <range/v3/algorithm/find_if_not.hpp>
#include <range/v3/view/view.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \addtogroup group-views
        /// @{
        template<typename Rng, typename Pred>
        struct remove_if_view
          : range_adaptor<remove_if_view<Rng, Pred>, Rng>
        {
        private:
            friend range_access;
            semiregular_invokable_t<Pred> pred_;
            optional<range_iterator_t<Rng>> begin_;

            struct adaptor
              : adaptor_base
            {
            private:
                remove_if_view *rng_;
                using adaptor_base::advance;
                void satisfy(range_iterator_t<Rng> &it) const
                {
                    it = find_if_not(std::move(it), ranges::end(rng_->mutable_base()),
                        std::ref(rng_->pred_));
                }
            public:
                adaptor() = default;
                adaptor(remove_if_view &rng)
                  : rng_(&rng)
                {}
                range_iterator_t<Rng> begin(remove_if_view &) const
                {
                    auto &beg = rng_->begin_;
                    if(!beg)
                    {
                        beg = ranges::begin(rng_->mutable_base());
                        this->satisfy(*beg);
                    }
                    return *beg;
                }
                void next(range_iterator_t<Rng> &it) const
                {
                    this->satisfy(++it);
                }
                CONCEPT_REQUIRES(BidirectionalIterable<Rng>())
                void prev(range_iterator_t<Rng> &it) const
                {
                    auto &&pred = rng_->pred_;
                    do --it; while(pred(*it));
                }
            };
            adaptor begin_adaptor()
            {
                return {*this};
            }
            // TODO: if end is a sentinel, it holds an unnecessary pointer back to
            // this range.
            adaptor end_adaptor()
            {
                return {*this};
            }
        public:
            remove_if_view() = default;
            remove_if_view(remove_if_view &&that)
              : range_adaptor_t<remove_if_view>(std::move(that))
              , pred_(std::move(that).pred_)
              , begin_{}
            {}
            remove_if_view(remove_if_view const &that)
              : range_adaptor_t<remove_if_view>(that)
              , pred_(that.pred_)
              , begin_{}
            {}
            remove_if_view(Rng && rng, Pred pred)
              : range_adaptor_t<remove_if_view>{std::forward<Rng>(rng)}
              , pred_(invokable(std::move(pred)))
              , begin_{}
            {}
            remove_if_view& operator=(remove_if_view &&that)
            {
                this->range_adaptor_t<remove_if_view>::operator=(std::move(that));
                pred_ = std::move(that).pred_;
                begin_.reset();
                return *this;
            }
            remove_if_view& operator=(remove_if_view const &that)
            {
                this->range_adaptor_t<remove_if_view>::operator=(that);
                pred_ = that.pred_;
                begin_.reset();
                return *this;
            }
        };

        namespace view
        {
            struct remove_if_fn
            {
            private:
                friend view_access;
                template<typename Pred>
                static auto bind(remove_if_fn remove_if, Pred pred)
                RANGES_DECLTYPE_AUTO_RETURN
                (
                    make_pipeable(std::bind(remove_if, std::placeholders::_1, protect(std::move(pred))))
                )
            public:
                template<typename Rng, typename Pred>
                using Concept = meta::and_<
                    InputIterable<Rng>,
                    IndirectInvokablePredicate1<Pred, range_iterator_t<Rng>>>;

                template<typename Rng, typename Pred,
                    CONCEPT_REQUIRES_(Concept<Rng, Pred>())>
                remove_if_view<Rng, Pred>
                operator()(Rng && rng, Pred pred) const
                {
                    return {std::forward<Rng>(rng), std::move(pred)};
                }
            #ifndef RANGES_DOXYGEN_INVOKED
                template<typename Rng, typename Pred,
                    CONCEPT_REQUIRES_(!Concept<Rng, Pred>())>
                void operator()(Rng &&, Pred) const
                {
                    CONCEPT_ASSERT_MSG(InputIterable<Rng>(),
                        "The first argument to view::remove_if must be a model of the "
                        "InputIterable concept");
                    CONCEPT_ASSERT_MSG(IndirectInvokablePredicate1<Pred, range_iterator_t<Rng>>(),
                        "The second argument to view::remove_if must be callable with "
                        "a value of the range, and the return type must be convertible "
                        "to bool");
                }
            #endif
            };

            /// \relates remove_if_fn
            /// \ingroup group-views
            namespace
            {
                constexpr auto&& remove_if = static_const<view<remove_if_fn>>::value;
            }
        }
        /// @}
    }
}

#endif
