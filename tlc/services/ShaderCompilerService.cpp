#include "services/ShaderCompiler.hpp"
#include "services/assetmanager/AssetManager.hpp"

#include "shaderc/shaderc.hpp"

namespace tlc
{
    static inline shaderc_shader_kind ShaderTypeToShaderKind(const ShaderCompiler::ShaderType& type) 
    {
        switch (type)
        {
        case ShaderCompiler::ShaderType::Vertex:
            return shaderc_glsl_vertex_shader;
        case ShaderCompiler::ShaderType::Fragment:
            return shaderc_glsl_fragment_shader;
        case ShaderCompiler::ShaderType::Compute:
            return shaderc_glsl_compute_shader;
        default:
            log::Error("ShaderTypeToShaderKind: unknown shader type");
            return shaderc_glsl_vertex_shader;
        }
    }

    class ShaderCompilerIncluder : public shaderc::CompileOptions::IncluderInterface
    {
    public:
        ShaderCompilerIncluder(Raw<ShaderCompiler> compiler)
        {
            m_Compiler = compiler;
        }

        virtual Raw<shaderc_include_result> GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override
        {
            auto include = m_Compiler->GetInclude(
                requested_source,
                requesting_source,
                static_cast<U32>(include_depth));

            Raw<shaderc_include_result> result = new shaderc_include_result();
            auto source_name = new char[include.first.size() + 1];
            if (source_name == nullptr)
            {
                log::Error("ShaderCompilerIncluder::GetInclude: failed to allocate memory for source_name");
                return nullptr;
            }
            std::memset(source_name, 0, include.first.size() + 1);
            std::copy(include.first.begin(), include.first.end(), source_name);
            result->source_name = source_name;
            result->source_name_length = include.first.size();            

            auto content = new char[include.second.size() + 1];
            if (content == nullptr)
            {
                log::Error("ShaderCompilerIncluder::GetInclude: failed to allocate memory for content");
                return nullptr;
            }
            std::memset(content, 0, include.second.size() + 1);
            std::copy(include.second.begin(), include.second.end(), content);
            result->content = content;
            result->content_length = include.second.size();

            result->user_data = this;
            return result;
        }


        virtual void ReleaseInclude(Raw<shaderc_include_result> data) override
        {
            delete[] data->source_name;
            delete[] data->content;
            delete data;
        }

        virtual ~ShaderCompilerIncluder() 
        {
            m_Compiler = nullptr;
        }

    private:
        Raw<ShaderCompiler> m_Compiler = nullptr;            
    };

    void ShaderCompiler::OnStart() 
    {

    }

    void ShaderCompiler::OnEnd()
    {

    }

    void ShaderCompiler::Setup() 
    {

    }

    String ShaderCompiler::Preprocess(const std::string& shaderSource, ShaderCompiler::ShaderType type, const String& inputFileName)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;    

        for (auto& [name, value] : m_Macros)
        {
            options.AddMacroDefinition(name, value);
        }
        options.SetSourceLanguage(shaderc_source_language_glsl);
        options.SetOptimizationLevel(shaderc_optimization_level(m_OptimizationLevel));
        if (!m_EnableWarnings) options.SetSuppressWarnings();
        if (m_EnableWarnings && m_WarningsAsErrors) options.SetWarningsAsErrors();       
        options.SetIncluder(CreateScope<ShaderCompilerIncluder>(this));

        auto result = compiler.PreprocessGlsl(shaderSource, ShaderTypeToShaderKind(type), inputFileName.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            log::Error("ShaderCompiler::PreprocessShader: failed to preprocess shader: {}", result.GetErrorMessage());
            return "";
        }

        if (m_EnableWarnings && result.GetNumWarnings() > 0)
        {
            log::Warn("ShaderCompiler::PreprocessShader: shader has warnings: {}", result.GetNumWarnings());
            for (U32 i = 0; i < result.GetNumWarnings(); i++)
            {
                log::Warn("warning: {}", result.GetErrorMessage());
            }
        }

        if (result.GetNumErrors() > 0)
        {
            log::Error("ShaderCompiler::PreprocessShader: shader has errors: {}", result.GetNumErrors());
            for (U32 i = 0; i < result.GetNumErrors(); i++)
            {
                log::Error("error: {}", result.GetErrorMessage());
            }
            return "";
        }

        return String(result.begin(), result.end());
    }

    String ShaderCompiler::ToAssembly(const std::string& shaderSource, ShaderCompiler::ShaderType type, const String& inputFileName)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;    

        for (auto& [name, value] : m_Macros)
        {
            options.AddMacroDefinition(name, value);
        }
        options.SetSourceLanguage(shaderc_source_language_glsl);
        options.SetOptimizationLevel(shaderc_optimization_level(m_OptimizationLevel));
        if (!m_EnableWarnings) options.SetSuppressWarnings();
        if (m_EnableWarnings && m_WarningsAsErrors) options.SetWarningsAsErrors();       
        options.SetIncluder(CreateScope<ShaderCompilerIncluder>(this));

        auto result = compiler.CompileGlslToSpvAssembly(shaderSource, ShaderTypeToShaderKind(type), inputFileName.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            log::Error("ShaderCompiler::PreprocessShader: failed to preprocess shader: {}", result.GetErrorMessage());
            return "";
        }

        if (m_EnableWarnings && result.GetNumWarnings() > 0)
        {
            log::Warn("ShaderCompiler::PreprocessShader: shader has warnings: {}", result.GetNumWarnings());
            for (U32 i = 0; i < result.GetNumWarnings(); i++)
            {
                log::Warn("warning: {}", result.GetErrorMessage());
            }
        }

        if (result.GetNumErrors() > 0)
        {
            log::Error("ShaderCompiler::PreprocessShader: shader has errors: {}", result.GetNumErrors());
            for (U32 i = 0; i < result.GetNumErrors(); i++)
            {
                log::Error("error: {}", result.GetErrorMessage());
            }
            return "";
        }

        return String(result.begin(), result.end());
    }

    List<U32> ShaderCompiler::ToSpv(const std::string& shaderSource, ShaderCompiler::ShaderType type, const String& inputFileName)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;    

        for (auto& [name, value] : m_Macros)
        {
            options.AddMacroDefinition(name, value);
        }
        options.SetSourceLanguage(shaderc_source_language_glsl);
        options.SetOptimizationLevel(shaderc_optimization_level(m_OptimizationLevel));
        if (!m_EnableWarnings) options.SetSuppressWarnings();
        if (m_EnableWarnings && m_WarningsAsErrors) options.SetWarningsAsErrors();       
        options.SetIncluder(CreateScope<ShaderCompilerIncluder>(this));

        auto result = compiler.CompileGlslToSpv(shaderSource, ShaderTypeToShaderKind(type), inputFileName.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            log::Error("ShaderCompiler::PreprocessShader: failed to preprocess shader: {}", result.GetErrorMessage());
            return List<U32>();
        }

        if (m_EnableWarnings && result.GetNumWarnings() > 0)
        {
            log::Warn("ShaderCompiler::PreprocessShader: shader has warnings: {}", result.GetNumWarnings());
            for (U32 i = 0; i < result.GetNumWarnings(); i++)
            {
                log::Warn("warning: {}", result.GetErrorMessage());
            }
        }

        if (result.GetNumErrors() > 0)
        {
            log::Error("ShaderCompiler::PreprocessShader: shader has errors: {}", result.GetNumErrors());
            for (U32 i = 0; i < result.GetNumErrors(); i++)
            {
                log::Error("error: {}", result.GetErrorMessage());
            }
            return List<U32>();
        }

        return List<U32>(result.begin(), result.end());
    }



    Pair<String, String> ShaderCompiler::GetInclude(const String& requestedSource, const String& requestingSource, U32 includeDepth)
    {
        auto assetManager = Services::GetService<AssetManager>();
        auto include = assetManager->GetAssetDataString(requestedSource);
        return { requestedSource, include };
    }

}