#pragma once

namespace Haketon
{
    /*
     * Implements a globally unique identifier.
     */
    struct FGuid
    {
    public:

        FGuid()
            : A(0)
            , B(0)
            , C(0)
            , D(0)
        { }
       
        explicit FGuid(uint32_t InA, uint32_t InB, uint32_t InC, uint32_t InD)
            : A(InA), B(InB), C(InC), D(InD)
        { }

        explicit FGuid(const std::string& InGuidStr)
        {
            if(!Parse(InGuidStr, *this))
            {
                Invalidate();
            }
        }

    public:

        friend bool operator==(const FGuid& X, const FGuid& Y)
        {
            return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) == 0;
        }

        friend bool operator!=(const FGuid& X, const FGuid& Y)
        {
            return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) != 0;
        }

        friend bool operator<(const FGuid& X, const FGuid& Y)
        {
            return	((X.A < Y.A) ? true : ((X.A > Y.A) ? false :
                    ((X.B < Y.B) ? true : ((X.B > Y.B) ? false :
                    ((X.C < Y.C) ? true : ((X.C > Y.C) ? false :
                    ((X.D < Y.D) ? true : ((X.D > Y.D) ? false : false))))))));
        }

        uint32_t& operator[](uint32_t Index)
        {
            HK_CORE_ASSERT(Index >= 0, "");
            HK_CORE_ASSERT(Index < 4, "");

            switch(Index)
            {
                case 0: return A;
                case 1: return B;
                case 2: return C;
                case 3: return D;
            }

            return A;
        }

        const uint32_t& operator[](uint32_t Index) const
        {
            HK_CORE_ASSERT(Index >= 0, "");
            HK_CORE_ASSERT(Index < 4, "");

            switch(Index)
            {
            case 0: return A;
            case 1: return B;
            case 2: return C;
            case 3: return D;
            }

            return A;
        }

    public:

        void Invalidate()
        {
            A = B = C = D = 0;
        }

        bool IsValid() const
        {
            return ((A | B | C | D) != 0);
        }

        std::string ToString() const;

    public:

        static FGuid NewGuid();

        static bool Parse(const std::string& GuidString, FGuid& OutGuid);

    private:

        uint32_t A;

        uint32_t B;

        uint32_t C;

        uint32_t D;
        
    };
}
