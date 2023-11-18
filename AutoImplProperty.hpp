//C++でC#の自動実装プロパティっぽいもの
//https://qiita.com/cmaru/items/443421c2f9ba8c652e64

#pragma once

#include <concepts>
#include <initializer_list>
#include <utility>
#include "TypeTraitUtil.hpp"
#include <functional>

namespace AutoImplProperty{

    template <class T>
    struct AutoImplAccesser{
        static T& Getter(const T& value){
            return value;
        }
        static void Setter(T& field, auto&& value){
            field = std::forward<T>(value);
        }
    };

    template <class T>
    struct NoneAccesser{
    };


    // 前方宣言
    template<class _Owner>
    class PropertyEnabled;



    template<class T>
    concept IsProperty = TypeTraitUtil::is_specialization_of_v< typename T::Outer, PropertyEnabled>;

    template<class T, template<class, template<class> class> class U>
    concept IsSameProperty = IsProperty<T> && TypeTraitUtil::is_specialization_property_of_v <T, U>;

    template<class T>
    concept IsReadonlyProperty = IsSameProperty<T, typename T::Outer::ReadonlyPublicGetPrivateSet>;

    template<class T>
    concept IsPrivateSetProperty = IsSameProperty<T, typename T::Outer::PublicGetPrivateSet> ;

    template<class T>
    concept IsProtectedSetProperty = IsSameProperty<T, typename T::Outer::PublicGetProtectedSet>;

    template<class T>
    concept IsInheritable = (!std::is_fundamental_v<T> );




    template<class _Owner>
    class PropertyEnabled{
    private:

        template <class _Inner>
        class MemberValueWrapper{
        public:
            using Owner = _Owner;
            using Inner = _Inner;
            using This = MemberValueWrapper<Inner>;

            friend PropertyEnabled<Owner>;

        private:
            Inner value;

        protected:
            template <class... Args>
                requires std::constructible_from<Inner, Args...>
            constexpr MemberValueWrapper(Args&&... args) : value(std::forward<Args>(args)...){
            }
            template <class T>
                requires std::constructible_from<Inner, std::initializer_list<T>>
            constexpr  MemberValueWrapper(std::initializer_list<T> initList) : value(initList){
            }

        public:
            //getter
            constexpr  operator const Inner& () const noexcept{
                return value;
            }

        protected:
            //setter
            template <class T>
            constexpr  This& operator=(T&& r)noexcept requires requires{value = std::forward<Inner>(r); }
            {
                this->value = std::forward<Inner>(r);
                return *this;
            }
            //setter
            template <class T>
            constexpr  This& operator=(T&& r)noexcept requires requires{value = std::forward<Inner>(r.value); }
            {
                this->value = std::forward<Inner>(r.value);
                return *this;
            }


            constexpr Inner& Get()  noexcept{
                return this->value;
            }
            constexpr const Inner& Get() const noexcept{
                return this->value;
            }

            //あとは利便性のために演算子オーバーロード

            This& operator++()requires requires{++value; }
            {
                ++this->value;
                return *this;
            }
            This& operator--()requires requires {--value; }
            {
                --this->value;
                return *this;
            }
            This operator++(int) requires requires {value++; }
            {
                auto temp = *this;
                ++this->value;
                return temp;
            }
            This operator--(int)requires requires {value--; }
            {
                auto temp = *this;
                --this->value;
                return temp;
            }
            template <class T>
            This& operator+=(T r)requires requires {value += r; }
            {
                this->value += r;
                return *this;
            }
            template <class T>
            This& operator-=(T r)requires requires {value -= r; }
            {
                this->value -= r;
                return *this;
            }
            template <class T>
            This& operator*=(T r)requires requires {value *= r; }
            {
                this->value *= r;
                return *this;
            }
            template <class T>
            This& operator/=(T r)requires requires {value /= r; }
            {
                this->value /= r;
                return *this;
            }
            template <class T>
            This& operator%=(T r)requires requires {value %= r; }
            {
                this->value %= r;
                return *this;
            }
            template <class T>
            This& operator<<=(T r)requires requires {value <<= r; }
            {
                this->value <<= r;
                return *this;
            }
            template <class T>
            This& operator>>=(T r)requires requires {value >>= r; }
            {
                this->value >>= r;
                return *this;
            }
        };


        template <class _Inner>
        struct Wrap;

        template <class _Inner>
            requires (!IsInheritable<_Inner>)
        struct Wrap<_Inner>{
            using Wrapped = typename MemberValueWrapper<_Inner>;
        };
        template <class _Inner>
            requires (IsInheritable<_Inner>)
        struct Wrap<_Inner>{
            using Wrapped = typename _Inner;

            static_assert(!std::is_final_v<_Inner>, "final class is not supperted.");
        };


        template <typename _Inner, template<typename...> typename _Accesser>
        class PropertyBase :public Wrap<_Inner>::Wrapped{
        public:
            using Inner = _Inner;
            using Wrapped = Wrap<_Inner>::Wrapped;
            using Accessered = _Accesser<Inner>;
            using This = PropertyBase<_Inner, _Accesser>;

            constexpr  operator const Inner& ()const noexcept{
                return Get();
            }

            //本体にアクセス
            constexpr const Inner& Get() const noexcept{
                return const_cast<This*>(this)->Get();
            }
            //メンバアクセス
            constexpr const Inner* operator->()const noexcept{
                return &Get();
            }
            //インデックスアクセス
            template<typename U>
            constexpr auto At(const U& idx)const{
                return ((const Inner&)Get())[idx];
            }

        protected:
            using Wrapped::Wrapped;


            template <class T>
            constexpr  This& operator=(T&& arg)noexcept
                requires requires(Inner& a){ a =Inner(std::forward<T>(arg)); }
            {
                Accessered::Setter(GetRaw(), Inner(std::forward<T>(arg)));
                return *this;
            }

            constexpr Inner& Get()  noexcept{
                return Accessered::Getter(GetRaw());
            }
            constexpr Inner& GetRaw()  noexcept{
                if constexpr (IsInheritable< Inner>)
                    return *this;
                else
                    return Wrapped::Get();
            }
            constexpr const Inner& GetRaw() const noexcept{
                return const_cast<This*>(this)->GetRaw();
            }



            This& operator++()requires requires(Inner v){ ++v; }
            {
                auto temp = Get();
                ++temp;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            This& operator--()requires requires (Inner v){ --v; }
            {
                auto temp = Get();
                --temp;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            Wrapped operator++(int) requires requires (Inner v){ v++; }
            {
                auto temp = Get();
                auto temp2 = temp++;
                Accessered::Setter(GetRaw(), temp);
                return temp2;
            }
            Wrapped operator--(int)requires requires (Inner v){ v--; }
            {
                auto temp = Get();
                auto temp2 = temp--;
                Accessered::Setter(GetRaw(), temp);
                return temp2;
            }
            template <class T>
            This& operator+=(T r)requires requires (Inner v){ v += r; }
            {
                auto temp = Get();
                temp += r;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            template <class T>
            This& operator-=(T r)requires requires(Inner v){ v -= r; }
            {
                auto temp = Get();
                temp -= r;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            template <class T>
            This& operator*=(T r)requires requires(Inner v){ v *= r; }
            {
                auto temp = Get();
                temp *= r;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            template <class T>
            This& operator/=(T r)requires requires(Inner v){ v /= r; }
            {
                auto temp = Get();
                temp /= r;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            template <class T>
            This& operator%=(T r)requires requires(Inner v){ v %= r; }
            {
                auto temp = Get();
                temp %= r;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            template <class T>
            This& operator<<=(T r)requires requires (Inner v){ v <<= r; }
            {
                auto temp = Get();
                temp <<= r;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }
            template <class T>
            This& operator>>=(T r)requires requires (Inner v){ v >>= r; }
            {
                auto temp = Get();
                temp >>= r;
                Accessered::Setter(GetRaw(), temp);
                return *this;
            }

        };

        template <typename _Inner>
        class PropertyBase<_Inner, NoneAccesser> :public Wrap<_Inner>::Wrapped{
        public:
            using Inner = _Inner;
            using Wrapped = Wrap<_Inner>::Wrapped;
            using This = PropertyBase<_Inner, NoneAccesser>;

            constexpr  operator const Inner& ()const noexcept{
                return this->Get();
            }

            //本体にアクセス
            constexpr const Inner& Get()const noexcept{
                return *this;
            }
            //メンバアクセス
            constexpr const Inner* operator->()const noexcept{
                return this;
            }
            //インデックスアクセス
            template<typename U>
            constexpr auto At(const U& idx)const{
                return ((const Inner&)*this)[idx];
            }

        protected:
            using Wrapped::Wrapped;
            using Wrapped::operator=;


            constexpr Inner& Get()  noexcept{
                if constexpr (IsInheritable< Inner>)
                    return *this;
                else
                    return Wrapped::Get();
            }
            constexpr const Inner& Get() const noexcept{
                return const_cast<This*>(this)->Get();
            }
        };

    public:

        template<class _Inner, template<class...> class _Accesser = NoneAccesser>
        class PublicGetPrivateSet;

        template<class _Inner, template<class...> class _Accesser >
            requires std::is_fundamental_v<_Inner>
        class PublicGetPrivateSet<_Inner,_Accesser> final :public PropertyBase<_Inner, _Accesser>{
        public:
            using Owner = _Owner;
            using Outer = PropertyEnabled<Owner>;
            using Inner = _Inner;
            using Base = PropertyBase<Inner, _Accesser>;

            friend typename std::enable_if<std::is_class<Owner>::value, Owner>::type;

        protected:
            using Base::Base;
            using Base::operator=;
        };

        /// <summary>
        /// PropertyBaseからprivate継承
        /// </summary>
        template <class _Inner, template<class...> class _Accesser >
            requires (!std::is_fundamental_v<_Inner>)
        class PublicGetPrivateSet<_Inner,_Accesser> final :private PropertyBase<_Inner, _Accesser>{
        public:
            using Owner = _Owner;
            using Outer = PropertyEnabled<Owner>;
            using Inner = _Inner;
            using Base = PropertyBase<Inner, _Accesser>;

            friend typename std::enable_if<std::is_class<Owner>::value, Owner>::type;

            using Base::operator->;
            using Base::Get;
            using Base::At;
        protected:
            using Base::Base;
            using Base::operator=;
        };


        //ProtectedSet版
        template<class _Inner, template<class...> class _Accesser = NoneAccesser>
        class PublicGetProtectedSet;


        template<class _Inner, template<class...> class _Accesser>
            requires (std::is_fundamental_v<_Inner>)
        class PublicGetProtectedSet<_Inner,_Accesser> final :public PropertyBase<_Inner, _Accesser>{
        public:
            using Owner = _Owner;
            using Outer = PropertyEnabled<Owner>;
            using Inner = _Inner;
            using Base = PropertyBase<Inner, _Accesser>;

            friend typename std::enable_if<std::is_class<Owner>::value, Owner>::type;
            friend PropertyEnabled<Owner>;

        protected:
            using Base::Base;
            using Base::operator=;
        };

        /// <summary>
        /// PropertyBaseからprivate継承
        /// </summary>
        template <class _Inner, template<class...> class _Accesser>
            requires (!std::is_fundamental_v<_Inner>)
        class PublicGetProtectedSet<_Inner,_Accesser> final :private PropertyBase<_Inner, _Accesser>{
        public:
            using Owner = _Owner;
            using Outer = PropertyEnabled<Owner>;
            using Inner = _Inner;
            using Base = PropertyBase<Inner, _Accesser>;

            friend typename std::enable_if<std::is_class<Owner>::value, Owner>::type;
            friend PropertyEnabled<Owner>;

            using Base::operator->;
            using Base::Get;
            using Base::At;
        protected:
            using Base::Base;
            using Base::operator=;
        };

    protected:

        //子クラスからProtectedSet版へのアクセス
        template <class T>
            requires std::same_as< _Owner, typename T::Owner>&& IsProtectedSetProperty<T>
        auto ProtectedAccess(T& target) -> T::Inner&{
            return target.Get();
        }
    };
}

////usage
//
//#include <vector>
//struct Fuga{
//public:
//   int a;
//};
//struct Hoge : public AutoImplProperty::PropertyEnabled<Hoge>{
//public:
//   // ≒ public int Num { get; private set; };
//   PublicGetPrivateSet<int, AutoImplProperty::NoneAccesser> Num;
//
//   // ≒ public List<int> Ints { get; private set; };
//   PublicGetPrivateSet<std::vector<int>, AutoImplProperty::AutoImplAccesser> Ints{ 1,3 };
//
//   // ≒ private List<int> _IntsC { get; set; };
//   //    public ReadOnlyCollection<int> IntsC => _IntsC;
//   PublicGetPrivateSet<std::vector<int>, AutoImplProperty::AutoImplAccesser> IntsR{ 3,4,5 };
//   PublicGetPrivateSet<Fuga, AutoImplProperty::AutoImplAccesser> FugaR;
//
//   //クラス内からのアクセス
//   void InnerAccess(){
//       //Num
//       int num = Num;                  //ok     暗黙の変換
//       Num = 4;                        //ok     代入
//       Num++;
//
//       //Ints
//       std::vector<int> ints = Ints;   //ok     暗黙の変換
//       //Ints
//       Ints.push_back(3);              //ok     非constメンバアクセス
//       Ints[0] = 2;                    //ok     非constインデックスアクセス
//       Ints = ints;                    //ok     代入
//
//       //IntsR
//       std::vector<int> intsC = IntsR; //ok     暗黙の変換
//       IntsR.push_back(2);             //ok     非constメンバアクセス
//       IntsR[0] = 2;                   //ok     非constインデックスアクセス
//       IntsR = intsC;                  //ok     代入
//
//       //FugaR
//       Fuga fugaR = FugaR; //ok     暗黙の変換
//       FugaR.a = 2;             //ok     非constメンバアクセス
//       FugaR = fugaR;                  //ok     代入
//   }
//};
////クラス外からのアクセス
//void main(){
//   Hoge h;
//
//   ////Num
//   //int num = h.Num;                  //ok     暗黙の変換
//   //h.Num = 4;                        //error  代入
//
//   ////Ints
//   //std::vector<int> ints = h.Ints;   //ok     暗黙の変換
//   //h.Ints.push_back(3);              //ok     非constメンバアクセス
//   //h.Ints[0] = 2;                    //ok     非constインデックスアクセス
//   //h.Ints = ints;                    //error  代入
//
//   ////IntsR
//   //std::vector<int> intsC = h.IntsR; //error  暗黙の変換 (private継承してるので関数を通さないとerror)
//
//   //intsC = h.IntsR.Get();            //ok     constアクセス(関数経由)
//   //h.IntsR->size();                  //ok     constメンバアクセス(演算子経由)
//   //auto s = h.IntsR.At(0);           //ok     constインデックスアクセス(関数経由)
//
//   //h.IntsR = intsC;                  //error  代入
//   //h.IntsR.push_back(2);             //error  非constメンバアクセス
//   //h.IntsR[0] = 2;                   //error  非constインデックスアクセス
//
//   //Fuga fugaR = h.FugaR.Get();             //error     暗黙の変換
//   // // h.FugaR.a = 2;                    //error     非constメンバアクセス
//   //  h.FugaR = fugaR;                  //error     代入
//
//
//   static_assert(!AutoImplProperty::IsProperty< Fuga>);
//   static_assert(!AutoImplProperty::IsProperty< Hoge>);
//   static_assert(!AutoImplProperty::IsProperty< AutoImplProperty::PropertyEnabled<Hoge>>);
//
//   static_assert(AutoImplProperty::IsProperty< decltype(h.FugaR)>);
//   static_assert(AutoImplProperty::IsPrivateSetProperty< decltype(h.FugaR)>);
//   static_assert(AutoImplProperty::IsReadonlyProperty< decltype(h.FugaR)>);
//
//   static_assert(!AutoImplProperty::IsProtectedSetProperty< decltype(h.FugaR)>);
//}


//namespace 概要 {
//    template<class Owner>
//    class PropertyEnabled {
//    private:
// 
//        //部分特殊化を使って同じ名前でどんな型でも使えるようにする
//        template<class _Inner>
//        class PropertyBase;
// 
//        /// <summary>
//        /// _Innerが基本型なのでメンバ変数にもつ
//        /// </summary>
//        template<class _Inner>
//        requires std::is_fundamental_v<_Inner>
//            class PropertyBase {
//            //...
//        };
//        /// <summary>
//        /// _Innerが基本型じゃないので継承する
//        ///</summary>
//        template <class _Inner>
//        requires (!std::is_fundamental_v<_Inner>)
//        class PropertyBase :public _Inner {
//            //...
//        };
//    public:
//
// 
//        template<class _Inner>
//        class PublicGetPrivateSet<_Inner> :public PropertyBase<_Inner> {
//            //...
//        };
//
//        /// <summary>
//        /// クラス外からのconstアクセス関数を実装
//        /// </summary>
//        template <class _Inner>
//        requires (!std::is_fundamental_v<_Inner>)
//        class ReadonlyPublicGetPrivateSet :private PropertyBase<_Inner> {
//            //...
//        };
//    };
//};