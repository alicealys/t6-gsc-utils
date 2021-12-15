#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "debug.hpp"
#include "gsc.hpp"
#include "json.hpp"

#include "scheduler.hpp"
#include "scripting.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace debug
{
    namespace
    {
        const game::dvar_t* developer_script = nullptr;

        // gsc-tool
        std::unordered_map<game::opcode, std::string> opcode_map
        {
            {game::opcode::OP_End, "END"},
            {game::opcode::OP_Return, "RETN"},
            {game::opcode::OP_GetUndefined, "GET_UNDEFINED"},
            {game::opcode::OP_GetZero, "GET_ZERO"},
            {game::opcode::OP_GetByte, "GET_BYTE"},
            {game::opcode::OP_GetNegByte, "GET_NBYTE"},
            {game::opcode::OP_GetUnsignedShort, "GET_USHORT"},
            {game::opcode::OP_GetNegUnsignedShort, "GET_NUSHORT"},
            {game::opcode::OP_GetInteger, "GET_INT"},
            {game::opcode::OP_GetFloat, "GET_FLOAT"},
            {game::opcode::OP_GetString, "GET_STRING"},
            {game::opcode::OP_GetIString, "GET_ISTRING"},
            {game::opcode::OP_GetVector, "GET_VECTOR"},
            {game::opcode::OP_GetLevelObject, "GET_LEVEL_OBJ"},
            {game::opcode::OP_GetAnimObject, "GET_ANIM_OBJ"},
            {game::opcode::OP_GetSelf, "GET_SELF"},
            {game::opcode::OP_GetLevel, "GET_LEVEL"},
            {game::opcode::OP_GetGame, "GET_GAME"},
            {game::opcode::OP_GetAnim, "GET_ANIM"},
            {game::opcode::OP_GetAnimation, "GET_ANIMATION"},
            {game::opcode::OP_GetGameRef, "GET_GAME_REF"},
            {game::opcode::OP_GetFunction, "GET_FUNCTION"},
            {game::opcode::OP_CreateLocalVariable, "CREATE_LOCAL_VARIABLE"},
            {game::opcode::OP_SafeCreateLocalVariables, "SAFE_CREATE_LOCAL_VARIABLES"},
            {game::opcode::OP_RemoveLocalVariables, "REMOVE_LOCAL_VARIABLES"},
            {game::opcode::OP_EvalLocalVariableCached, "EVAL_LOCAL_VARIABLE_CACHED"},
            {game::opcode::OP_EvalArray, "EVAL_ARRAY"},
            {game::opcode::OP_EvalLocalArrayRefCached, "EVAL_LOCAL_ARRAY_REF_CACHED"},
            {game::opcode::OP_EvalArrayRef, "EVAL_ARRAY_REF"},
            {game::opcode::OP_ClearArray, "CLEAR_ARRAY"},
            {game::opcode::OP_EmptyArray, "EMPTY_ARRAY"},
            {game::opcode::OP_GetSelfObject, "GET_SELF_OBJECT"},
            {game::opcode::OP_EvalFieldVariable, "EVAL_FIELD_VARIABLE"},
            {game::opcode::OP_EvalFieldVariableRef, "EVAL_FIELD_VARIABLE_REF"},
            {game::opcode::OP_ClearFieldVariable, "CLEAR_FIELD_VARIABLE"},
            {game::opcode::OP_SafeSetVariableFieldCached, "SAFE_SET_VARIABLE_FIELD_CACHED"},
            {game::opcode::OP_SafeSetWaittillVariableFieldCached, "SAFE_SET_WAITTILL_VARIABLE_FIELD_CACHED"},
            {game::opcode::OP_ClearParams, "CLEAR_PARAMS"},
            {game::opcode::OP_CheckClearParams, "CHECK_CLEAR_PARAMS"},
            {game::opcode::OP_EvalLocalVariableRefCached, "EVAL_LOCAL_VARIABLE_REF_CACHED"},
            {game::opcode::OP_SetVariableField, "SET_VARIABLE_FIELD"},
            {game::opcode::OP_CallBuiltin, "CALL_BUILTIN_FUNC"},
            {game::opcode::OP_CallBuiltinMethod, "CALL_BUILTIN_METHOD"},
            {game::opcode::OP_Wait, "WAIT"},
            {game::opcode::OP_WaitTillFrameEnd, "WAITTILLFRAMEEND"},
            {game::opcode::OP_PreScriptCall, "PRE_CALL"},
            {game::opcode::OP_ScriptFunctionCall, "SCRIPT_FUNC_CALL"},
            {game::opcode::OP_ScriptFunctionCallPointer, "SCRIPT_FUNC_CALL_POINTER"},
            {game::opcode::OP_ScriptMethodCall, "SCRIPT_METHOD_CALL"},
            {game::opcode::OP_ScriptMethodCallPointer, "SCRIPT_METHOD_CALL_POINTER"},
            {game::opcode::OP_ScriptThreadCall, "SCRIPT_THREAD_CALL"},
            {game::opcode::OP_ScriptThreadCallPointer, "SCRIPT_THREAD_CALL_POINTER"},
            {game::opcode::OP_ScriptMethodThreadCall, "SCRIPT_METHOD_THREAD_CALL"},
            {game::opcode::OP_ScriptMethodThreadCallPointer, "SCRIPT_METHOD_THREAD_CALL_POINTER"},
            {game::opcode::OP_DecTop, "DEC_TOP"},
            {game::opcode::OP_CastFieldObject, "CAST_FIELD_OBJECT"},
            {game::opcode::OP_CastBool, "CAST_BOOL"},
            {game::opcode::OP_BoolNot, "BOOL_NOT"},
            {game::opcode::OP_BoolComplement, "BOOL_COMPLEMENT"},
            {game::opcode::OP_JumpOnFalse, "JMP_FALSE"},
            {game::opcode::OP_JumpOnTrue, "JMP_TRUE"},
            {game::opcode::OP_JumpOnFalseExpr, "JMP_EXPR_FALSE"},
            {game::opcode::OP_JumpOnTrueExpr, "JMP_EXPR_TRUE"},
            {game::opcode::OP_Jump, "JMP"},
            {game::opcode::OP_JumpBack, "JMP_BACK"},
            {game::opcode::OP_Inc, "INC"},
            {game::opcode::OP_Dec, "DEC"},
            {game::opcode::OP_Bit_Or, "BIT_OR"},
            {game::opcode::OP_Bit_Xor, "BIT_XOR"},
            {game::opcode::OP_Bit_And, "BIT_AND"},
            {game::opcode::OP_Equal, "EQUAL"},
            {game::opcode::OP_NotEqual, "NOT_EQUAL"},
            {game::opcode::OP_LessThan, "LESS"},
            {game::opcode::OP_GreaterThan, "GREATER"},
            {game::opcode::OP_LessThanOrEqualTo, "LESS_EQUAL"},
            {game::opcode::OP_GreaterThanOrEqualTo, "GREATER_EQUAL"},
            {game::opcode::OP_ShiftLeft, "SHIFT_LEFT"},
            {game::opcode::OP_ShiftRight, "SHIFT_RIGHT"},
            {game::opcode::OP_Plus, "PLUS"},
            {game::opcode::OP_Minus, "MINUS"},
            {game::opcode::OP_Multiply, "MULT"},
            {game::opcode::OP_Divide, "DIV"},
            {game::opcode::OP_Modulus, "MOD"},
            {game::opcode::OP_SizeOf, "SIZE"},
            {game::opcode::OP_WaitTillMatch, "WAITTILLMATCH"},
            {game::opcode::OP_WaitTill, "WAITTILL"},
            {game::opcode::OP_Notify, "NOTIFY"},
            {game::opcode::OP_EndOn, "ENDON"},
            {game::opcode::OP_VoidCodePos, "VOIDCODEPOS"},
            {game::opcode::OP_Switch, "SWITCH"},
            {game::opcode::OP_EndSwitch, "ENDSWITCH"},
            {game::opcode::OP_Vector, "VECTOR"},
            {game::opcode::OP_GetHash, "GET_HASH"},
            {game::opcode::OP_RealWait, "REAL_WAIT"},
            {game::opcode::OP_VectorConstant, "VECTOR_CONSTANT"},
            {game::opcode::OP_IsDefined, "IS_DEFINED"},
            {game::opcode::OP_VectorScale, "VECTOR_SCALE"},
            {game::opcode::OP_AnglesToUp, "ANGLES_TO_UP"},
            {game::opcode::OP_AnglesToRight, "ANGLES_TO_RIGHT"},
            {game::opcode::OP_AnglesToForward, "ANGLES_TO_FORDWARD"},
            {game::opcode::OP_AngleClamp180, "ANGLE_CLAMP_180"},
            {game::opcode::OP_VectorToAngles, "VECTOR_TO_ANGLES"},
            {game::opcode::OP_Abs, "ABS"},
            {game::opcode::OP_GetTime, "GET_TIME"},
            {game::opcode::OP_GetDvar, "GET_DVAR"},
            {game::opcode::OP_GetDvarInt, "GET_DVAR_INT"},
            {game::opcode::OP_GetDvarFloat, "GET_DVAR_FLOAT"},
            {game::opcode::OP_GetDvarVector, "GET_DVAR_VECTOR"},
            {game::opcode::OP_GetDvarColorRed, "GET_DVAR_COLOR_RED"},
            {game::opcode::OP_GetDvarColorGreen, "GET_DVAR_COLOR_GREEN"},
            {game::opcode::OP_GetDvarColorBlue, "GET_DVAR_COLOR_BLUE"},
            {game::opcode::OP_GetDvarColorAlpha, "GET_DVAR_COLOR_ALPHA"},
            {game::opcode::OP_FirstArrayKey, "FIRST_ARRAY_KEY"},
            {game::opcode::OP_NextArrayKey, "NEXT_ARRAY_KEY"},
            {game::opcode::OP_ProfileStart, "PROFILE_START"},
            {game::opcode::OP_ProfileStop, "PROFILE_STOP"},
            {game::opcode::OP_SafeDecTop, "SAFE_DEC_TOP"},
            {game::opcode::OP_Nop, "NOP"},
            {game::opcode::OP_Abort, "ABORT"},
            {game::opcode::OP_Object, "OBJECT"},
            {game::opcode::OP_ThreadObject, "THREAD_OBJECT"},
            {game::opcode::OP_EvalLocalVariable, "EVAL_LOCAL_VARIABLE"},
            {game::opcode::OP_EvalLocalVariableRef, "EVAL_LOCAL_VARIABLE_REF"},
            {game::opcode::OP_DevblockBegin, "DEVBLOCK_BEGIN"},
            {game::opcode::OP_DevblockEnd, "DEVBLOCK_END"},
            {game::opcode::OP_Breakpoint, "BREAKPOINT"},
        };

        std::string get_opcode_name(const game::opcode opcode)
        {
            if (opcode_map.find(opcode) != opcode_map.end())
            {
                return opcode_map[opcode];
            }

            return "unknown";
        }

        void print_error(const game::opcode opcode)
        {
            if (!developer_script->current.enabled)
            {
                return;
            }

            const auto error = reinterpret_cast<const char*>(SELECT(0x2E27C70, 0x2DF7F70));
            const auto fs_pos = *reinterpret_cast<char**>(SELECT(0x2E23C08, 0x2DF3F08));

            if (opcode == game::opcode::OP_CallBuiltin || opcode == game::opcode::OP_CallBuiltinMethod)
            {
                std::string name{};
                for (auto i = 0; i < 4; i++)
                {
                    const auto ptr = *reinterpret_cast<void**>(fs_pos + i);
                    name = opcode == game::opcode::OP_CallBuiltinMethod
                        ? gsc::find_builtin_method_name(ptr) 
                        : gsc::find_builtin_name(ptr);

                    if (!name.empty())
                    {
                        break;
                    }
                }

                const auto type = opcode == game::opcode::OP_CallBuiltinMethod 
                    ? "method" 
                    : "function";

                printf("******* script runtime error *******\n");
                printf("in call to builtin %s \"%s\": %s\n", type, name.data(), error);
                printf(debug::get_call_stack().data());
                printf("************************************\n");
            }
            else
            {
                const auto opcode_name = get_opcode_name(opcode);

                printf("******* script runtime error *******\n");
                printf("while processing instruction %s: %s\n", opcode_name.data(), error);
                printf(debug::get_call_stack().data());
                printf("************************************\n");
            }
        }

        __declspec(naked) void vm_exeucte_error_stub_zm()
        {
            __asm
            {
                pushad
                push eax
                call print_error
                pop eax
                popad

                add eax, 0xFFFFFFE5
                mov [ebp + 0x6C], esi
                mov [ebp + 0x44], edx

                push 0x8F77C0
                retn
            }
        }

        __declspec(naked) void vm_exeucte_error_stub_mp()
        {
            __asm
            {
                pushad
                push eax
                call print_error
                pop eax
                popad

                add eax, 0xFFFFFFE5
                mov[ebp + 0x6C], esi
                mov[ebp + 0x44], edx

                push 0x8F8A60
                retn
            }
        }

        utils::hook::detour scr_terminal_error_hook;
        void scr_terminal_error_stub(int inst, const char* error)
        {
            printf("====================================================\n");
            printf("Scr_TerminalError: %s\n", error);
            printf("====================================================\n");
            scr_terminal_error_hook.invoke<void>(inst, error);
        }
    }

    std::vector<scripting::thread> get_all_threads()
    {
        std::vector<scripting::thread> threads;

        for (auto i = 1; i < 0x8000; i++)
        {
            const auto type = game::scr_VarGlob->objectVariableValue[i].w.type & 0x7F;
            if (type == game::SCRIPT_THREAD || type == game::SCRIPT_TIME_THREAD || type == game::SCRIPT_NOTIFY_THREAD)
            {
                threads.push_back(i);
            }
        }

        return threads;
    }

    unsigned int get_var_count()
    {
        auto count = 0;
        for (auto i = 0; i < 0x8000; i++)
        {
            const auto type = game::scr_VarGlob->objectVariableValue[i].w.type & 0x7F;
            count += type != game::SCRIPT_FREE;
        }
        return count;
    }

    unsigned int get_child_var_count()
    {
        auto count = 0;
        for (auto i = 0; i < 0x10000; i++)
        {
            const auto type = game::scr_VarGlob->childVariableValue[i].type & 0x7F;
            count += type != game::SCRIPT_FREE;
        }
        return count;
    }

    std::string get_call_stack(bool print_local_vars)
    {
        std::string info{};
        const auto line = [&info](const std::string& text)
        {
            info.append(text);
            info.append("\r\n");
        };

        const auto fs_pos = *reinterpret_cast<char**>(SELECT(0x2E23C08, 0x2DF3F08));

        for (auto frame = game::scr_VmPub->function_frame; frame != game::scr_VmPub->function_frame_start; --frame)
        {
            auto pos = frame == game::scr_VmPub->function_frame
                ? fs_pos
                : frame->fs.pos;
            auto function = scripting::find_function_pair(pos);

            if (!function.has_value() && pos != frame->fs.pos)
            {
                pos = frame->fs.pos;
                function = scripting::find_function_pair(frame->fs.pos);
            }

            if (function.has_value())
            {
                const auto value = function.value();
                line(utils::string::va("\tat function \"%s\" in file \"%s.gsc\"", value.second.data(), value.first.data()));
            }
            else
            {
                line(utils::string::va("\tat unknown location (%p)", pos));
            }

            const scripting::object local_vars{frame->fs.localId};
            if (local_vars.get_keys().size() && print_local_vars)
            {
                line(utils::string::va("\t\tlocal vars: %s", json::gsc_to_string(local_vars).data()));
            }
        }

        return info;
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            developer_script = game::Dvar_FindVar("developer_script");
            utils::hook::jump(SELECT(0x8F8A57, 0x8F77B7), SELECT(vm_exeucte_error_stub_mp, vm_exeucte_error_stub_zm));

            scr_terminal_error_hook.create(SELECT(0x698C50, 0x410440), scr_terminal_error_stub);

            gsc::function::add("crash", [](const gsc::function_args& args) -> scripting::script_value
            {
                if (!developer_script->current.enabled)
                {
                    return {};
                }

                *reinterpret_cast<int*>(0) = 1;
                return {};
            });

            gsc::function::add("breakpoint", [](const gsc::function_args& args) -> scripting::script_value
            {
                if (!developer_script->current.enabled)
                {
                    return {};
                }

                std::string msg;
                if (args.size() >= 1)
                {
                    msg = args[0].to_string();
                }

                const auto text = utils::string::va("%s\n %s", msg.data(), get_call_stack().data());
                MessageBoxA(nullptr, text, "GSC Breakpoint", MB_ICONERROR);
                return {};
            });

            gsc::function::add("assert", [](const gsc::function_args& args) -> scripting::script_value
            {
                if (!developer_script->current.enabled )
                {
                    return {};
                }

                const auto assertion = args[0].as<bool>();
                if (!assertion)
                {
                    throw std::runtime_error("assertion failed");
                }

                return {};
            });

            gsc::function::add("killthread", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto function = args[0].as<scripting::function>();
                const auto threads = get_all_threads();

                auto entity_id = 0;
                if (args.size() >= 2)
                {
                    entity_id = args[1].as<scripting::entity>().get_entity_id();
                }

                for (const auto& thread : threads)
                {
                    if (thread.get_start_pos() == function.get_pos() && (!entity_id || (entity_id == thread.get_self())))
                    {
                        thread.kill();
                        return 1;
                    }
                }

                return 0;
            });

            gsc::function::add("killallthreads", [](const gsc::function_args& args) -> scripting::script_value
            {
                const auto function = args[0].as<scripting::function>();
                const auto threads = get_all_threads();

                auto entity_id = 0;
                if (args.size() >= 2)
                {
                    entity_id = args[1].as<scripting::entity>().get_entity_id();
                }

                auto count = 0;
                for (const auto& thread : threads)
                {
                    if (thread.get_start_pos() == function.get_pos() && (!entity_id || (entity_id == thread.get_self())))
                    {
                        thread.kill();
                        count++;
                    }
                }

                return count;
            });

            gsc::function::add("getvarusage", [](const gsc::function_args& args) -> scripting::script_value
            {
                return get_var_count();
            });

            gsc::function::add("getchildvarusage", [](const gsc::function_args& args) -> scripting::script_value
            {
                return get_child_var_count();
            });

            gsc::function::add("getusagestats", [](const gsc::function_args& args) -> scripting::script_value
            {
                scripting::object stats{};

                stats["maxvars"] = 0x8000;
                stats["maxchildvars"] = 0x10000;
                stats["childvars"] = get_child_var_count();
                stats["vars"] = get_var_count();

                return stats;
            });
        }
    };
}

REGISTER_COMPONENT(debug::component)
