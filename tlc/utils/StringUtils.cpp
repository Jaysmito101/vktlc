#include "utils/StringUtils.hpp"
#include <cctype>

namespace tlc
{
    namespace utils
    {
        F32 LevenshteinMatch(const String &str1, const String &str2)
        {
            if (str1.empty() && str2.empty()) return 1.0f;
            
            int n = static_cast<int>(str1.length());
            int m = static_cast<int>(str2.length());
            
            if (n == 0 || m == 0) return 0.0f;
            
            std::vector<int> p(n + 1);
            std::vector<int> d(n + 1);
            
            for (int i = 0; i <= n; i++) {
                p[i] = i;
            }
            
            for (int j = 1; j <= m; j++) {
                char t_j = str2[j - 1];
                d[0] = j;
                
                for (int i = 1; i <= n; i++) {
                    int cost = (std::tolower(str1[i - 1]) == std::tolower(t_j)) ? 0 : 1;
                    d[i] = std::min({
                        d[i - 1] + 1,
                        p[i] + 1,
                        p[i - 1] + cost
                    });
                }
                
                std::swap(p, d);
            }
            
            int distance = p[n];
            
            F32 maxLength = static_cast<F32>(std::max(n, m));
            return 1.0f - (static_cast<F32>(distance) / maxLength);
        }

        F32 LevenshteinSubstringMatch(const String& main, const String& searchTerm)
        {
            if (searchTerm.empty()) return 1.0f;
            if (main.empty()) return 0.0f;
            
            Size n = main.length();
            Size m = searchTerm.length();
            
            if (m > n) return LevenshteinMatch(main, searchTerm);
            
            std::vector<std::vector<I32>> dp(m + 1, std::vector<I32>(n + 1, 0));
            
            for (Size j = 0; j <= n; j++) {
                dp[0][j] = 0;
            }
            
            for (Size i = 1; i <= m; i++) {
                dp[i][0] = (I32)i;
            }
            
            for (Size i = 1; i <= m; i++) {
                for (Size j = 1; j <= n; j++) {
                    I32 cost = (std::tolower(searchTerm[i - 1]) == std::tolower(main[j - 1])) ? 0 : 1;
                    
                    dp[i][j] = std::min({
                        dp[i - 1][j] + 1,
                        dp[i][j - 1] + 1,
                        dp[i - 1][j - 1] + cost
                    });
                }
            }
            
            int minDistance = dp[m][0];
            for (Size j = 1; j <= n; j++) {
                minDistance = std::min(minDistance, dp[m][j]);
            }
            
            if (n >= m) {
                for (Size i = 0; i <= n - m; i++) {
                    bool isMatch = true;
                    for (Size k = 0; k < m; k++) {
                        if (std::tolower(main[i + k]) != std::tolower(searchTerm[k])) {
                            isMatch = false;
                            break;
                        }
                    }
                    if (isMatch) {
                        return 1.0f;
                    }
                }
            }
            
            F32 maxLength = static_cast<F32>(m);
            return 1.0f - (static_cast<F32>(minDistance) / maxLength);
        }
    }
}