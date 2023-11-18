
#include <iostream>
#include "AutoImplProperty.hpp"
#include <vector>
#include <utility>
#include <memory>

#include <type_traits>
#include <array>

struct TestFuga{
public:
    std::vector<int> ef;
    int a;
    int asfefs;
    TestFuga(int aa) :a(aa){
    }
    TestFuga& operator++(){
        ++this->a;
        return *this;
    }
    //TestFuga() = default;
    //TestFuga(TestFuga&&) = default;

    //TestFuga& operator=(TestFuga&&) = default;
};
struct TestFuga2{
public:
    int a;
    //TestFuga2() = default;
    //TestFuga2(TestFuga2&&) = default;

    //TestFuga2& operator=(TestFuga2&&) = default;
};

struct TestHoge : public AutoImplProperty::PropertyEnabled<TestHoge>{
public:

    template <class T>
    struct AddAccesser{
        static  T& Getter(const T& value){
            return value + 10;
        }
        static void Setter(T& field, auto&& value){
            field = std::forward<T>(value + 5);
        }
    };
    // ≒ public int ProtectedNum { get; protected set; };
    PublicGetProtectedSet<int, AddAccesser> ProtectedNum = 3;


    // ≒ public int Num { get; private set; };
    PublicGetPrivateSet<int, AutoImplProperty::AutoImplAccesser> Num{ 3 };


    // ≒ private List<int> _IntsR { get; set; };
    //    public ReadOnlyCollection<int> IntsR => _IntsR;
    PublicGetPrivateSet<std::vector<int>> IntsR{ 3,4,5 };

    // ≒ public int ProtectedNum_ { get; protected set; };
    //    public ReadOnlyFuga ProtectedNum => _ProtectedNum;
    PublicGetProtectedSet<TestFuga> ProtectedFugaR{ 4 };

    TestHoge() :ProtectedFugaR(4){
    }


    //クラス内からのアクセステスト
    void InnerAccessTest(){
        //Num
        int num = Num;                  //ok     暗黙の変換
        Num = 4;                        //ok     代入
        ++Num;
        --Num;
        Num++;
        Num--;
        Num += 4;
        Num *= 2;
        Num /= 2;
        Num -= 2;
        Num %= 2;
        Num << 2;
        Num >> 2;
        // PublicGetPrivateSet<bool, AutoImplProperty::AutoImplAccesser> bb{ };
         //bb = 2;
         //bb += 2;
         //bb -= 2;

         //IntsR
        std::vector<int> intsC = IntsR; //ok     暗黙の変換
        IntsR.push_back(2);             //ok     非constメンバアクセス
        IntsR[0] = 2;                   //ok     非constインデックスアクセス
        IntsR = intsC;                  //ok     代入

        PublicGetProtectedSet<TestFuga> s{ 5 };
        //FugaR
        //TestFuga fugaR = ProtectedFugaR;//ok     暗黙の変換
        TestFuga fugfaR = std::move(ProtectedFugaR);//ok     暗黙の変換
        s = std::move(ProtectedFugaR);//ok     暗黙の変換
        //ProtectedFugaR.a = 2;           //ok     非constメンバアクセス
        //ProtectedFugaR = fugaR;         //ok     代入


        ProtectedNum = ProtectedNum;
        ProtectedNum = std::move(ProtectedNum);
        ProtectedNum++;

        auto a = ProtectedNum;
        int aaww = a + ProtectedNum;
        //ProtectedFugaR = ProtectedFugaR; // error

        PublicGetProtectedSet<TestFuga2> ProtectedFugaR2{};
        TestFuga2 f2 = std::move(ProtectedFugaR2);
        ProtectedFugaR2 = std::move(ProtectedFugaR2);
        ProtectedFugaR2 = f2;

        //f2 = ProtectedFugaR2;          // error

        //++ProtectedFugaR;

        PublicGetPrivateSet<float> c;
        //c << 2;                        // error
        c = c + c;
        c++;
        ++c;
    }
};

struct TestHogeHoge :public TestHoge{
public:

    void InheritanceAccessTest(){
        int num = ProtectedNum;

        const int& refNum = ProtectedNum;
        //ProtectedNum = 2;   // error
        ProtectedAccess(ProtectedNum) = 2;
        ProtectedAccess(ProtectedFugaR).a = 3;
    }

};


int main(){
    std::cout << "Hello World!\n";
    TestHogeHoge hghg;
    hghg.InnerAccessTest();
    hghg.InheritanceAccessTest();

}