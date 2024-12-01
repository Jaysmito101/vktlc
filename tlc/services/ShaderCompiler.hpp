#include "core/Core.hpp"
#include "services/Services.hpp"


namespace tlc 
{

    class ShaderCompiler : public IService
    {
    public:

        enum class ShaderType
        {
            Vertex,
            Fragment,
            Compute,
        };

        inline void ClearMacros() { m_Macros.clear(); }
        inline void AddMacro(const String& name, const String& value) { m_Macros.push_back({ name, value }); }
        inline void SetOptimizationLevel(U32 level) { m_OptimizationLevel = level; }
        inline void EnableWarnings(Bool enable) { m_EnableWarnings = enable; }
        inline void SetWarningsAsErrors(Bool enable) { m_WarningsAsErrors = enable; }
        inline void DisableWarnings() { m_EnableWarnings = false; }

        String Preprocess(const std::string& shaderSource, ShaderCompiler::ShaderType type, const String& inputFileName = "_ShaderMain");
        String ToAssembly(const std::string& shaderSource, ShaderCompiler::ShaderType type, const String& inputFileName = "_ShaderMain");
        List<U32> ToSpv(const std::string& shaderSource, ShaderCompiler::ShaderType type, const String& inputFileName = "_ShaderMain");

        void Setup();
        virtual void OnStart() override;
        virtual void OnEnd() override;

        // We do not care about include type (relative "" or standard <>) we only use relative
        Pair<String, String> GetInclude(const String& requestedSource, const String& requestingSource, U32 includeDepth);


    private:
        List<Pair<String, String>> m_Macros;
        U32 m_OptimizationLevel = 0;
        Bool m_EnableWarnings = true;
        Bool m_WarningsAsErrors = false;
        std::mutex m_Mutex;
    };
}