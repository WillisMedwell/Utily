#include <gtest/gtest.h>

#include "Utily/StaticVector.hpp"

#include <iterator>

TEST(StaticVector, Constructors) {
    { // default
        Utily::StaticVector<int, 10> sv;
        EXPECT_EQ(sv.capacity(), 10);
        EXPECT_EQ(sv.size(), 0);
        EXPECT_EQ(sv.begin(), sv.end());
    }

    { // default
        const Utily::StaticVector<int, 10> sv;
        EXPECT_EQ(sv.capacity(), 10);
        EXPECT_EQ(sv.size(), 0);
        EXPECT_EQ(sv.begin(), sv.end());
    }

    { // with args&&
        Utily::StaticVector<int, 10> sv { 1, 2, 3, 4 };
        EXPECT_EQ(sv.capacity(), 10);
        EXPECT_EQ(sv.size(), 4);
        EXPECT_NE(sv.begin(), sv.end());
        EXPECT_EQ(std::distance(sv.begin(), sv.end()), 4);

        auto iter = sv.begin();
        EXPECT_EQ(iter, sv.begin());
        EXPECT_EQ(*(iter++), 1);
        EXPECT_EQ(*(iter++), 2);
        EXPECT_EQ(*(iter++), 3);
        EXPECT_EQ(*(iter++), 4);
        EXPECT_EQ(iter, sv.end());
    }
}

TEST(StaticVector, PushOperations) {
    { // copy push
        Utily::StaticVector<std::string, 10> sv {};
        std::string a = "10";
        sv.push_back(a);
        EXPECT_EQ(sv.front(), "10");
        EXPECT_EQ(sv[0], "10");
        EXPECT_EQ(*sv.begin(), "10");
        EXPECT_EQ(sv.size(), 1);
    }

    { // move push
        Utily::StaticVector<std::string, 10> sv {};
        sv.push_back(std::string { "10" });
        EXPECT_EQ(sv.front(), "10");
        EXPECT_EQ(sv[0], "10");
        EXPECT_EQ(*sv.begin(), "10");
        EXPECT_EQ(sv.size(), 1);
    }

    { // emplace back
        Utily::StaticVector<std::string, 10> sv {};
        sv.emplace_back("10");
        sv.emplace_back("20");
        EXPECT_EQ(sv.front(), "10");
        EXPECT_EQ(sv.back(), "20");
        EXPECT_EQ(sv[0], "10");
        EXPECT_EQ(*sv.begin(), "10");
        EXPECT_EQ(*std::next(sv.begin()), "20");
        EXPECT_EQ(sv.size(), 2);
        EXPECT_EQ(std::distance(sv.begin(), sv.end()), 2);
    }
}

TEST(StaticVector, ProperDestruction) {
    static int default_constructions = 0;
    static int copy_constructions = 0;
    static int move_constructions = 0;

    static int move_operator = 0;
    static int copy_operator = 0;

    static int destructions = 0;

    auto reset = [&]() {
        default_constructions = 0;
        copy_constructions = 0;
        move_constructions = 0;

        move_operator = 0;
        copy_operator = 0;

        destructions = 0;
    };

    [[maybe_unused]] auto print_state = [&]() {
        std::cout
            << "{}      " << default_constructions << '\n'
            << "copy {} " << copy_constructions << '\n'
            << "move {} " << move_constructions << '\n'
            << "move =  " << move_operator << '\n'
            << "copy =  " << copy_operator << '\n'
            << "~{}     " << destructions << std::endl;
    };

    struct DestructorChecker {
        DestructorChecker() {
            ++default_constructions;
        }
        DestructorChecker(DestructorChecker&&) {
            ++move_constructions;
        }
        DestructorChecker(const DestructorChecker&) {
            ++copy_constructions;
        }
        DestructorChecker& operator=(DestructorChecker&& other [[maybe_unused]]) {
            ++move_operator;
            return *this;
        }
        DestructorChecker& operator=(const DestructorChecker& other [[maybe_unused]]) {
            ++copy_operator;
            return *this;
        }

        ~DestructorChecker() {
            ++destructions;
        }
    };

    { // ensure that types aren't constructed when container is declared.
        reset();
        Utily::StaticVector<DestructorChecker, 10> sv {};
        EXPECT_EQ(default_constructions + copy_constructions + move_constructions, 0);
        EXPECT_EQ(destructions, 0);
    }

    { // test basic emplace.
        reset();
        {
            Utily::StaticVector<DestructorChecker, 10> sv {};
            sv.emplace_back();
            sv.push_back();
        }
        EXPECT_EQ(default_constructions, 2);
        EXPECT_EQ(destructions, 2);
    }

    { // emplace/push moved types.
        reset();
        {
            Utily::StaticVector<DestructorChecker, 10> sv {};
            sv.emplace_back(DestructorChecker {});
            sv.push_back(DestructorChecker {});
        }
        EXPECT_EQ(default_constructions, 2);
        EXPECT_EQ(move_constructions, 2);
        EXPECT_EQ(destructions, 4);
    }

    { // emplace/push copy types.
        reset();
        {
            Utily::StaticVector<DestructorChecker, 10> sv {};
            DestructorChecker dc { };
            sv.emplace_back(dc);
            sv.push_back(dc);
        }
        EXPECT_EQ(default_constructions, 1);
        EXPECT_EQ(copy_constructions, 2);
        EXPECT_EQ(destructions, 3);
    }

    { // intialiser list
        reset();
        {
            Utily::StaticVector<DestructorChecker, 10> sv { DestructorChecker {}, DestructorChecker {}, DestructorChecker {} };
        }
        EXPECT_EQ(default_constructions, 3);
        EXPECT_EQ(copy_constructions, 3);
        EXPECT_EQ(destructions, 6);
    }
}