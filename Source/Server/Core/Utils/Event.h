/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

// This file contains a very basic set of types for primitive
// event registering and dispatch.
// 
// You can use it roughly like this:
// 
//   using MyDelegate = Delegate<int>;
//   using MyEvent = Event<MyDelegate>;
// 
//   MyEvent OnSomething;
//   MyEvent::DelegatePtr ptr = OnSomething.Register(Callback);
//   OnSomething.Broadcast(1);
//

#pragma once

#include <functional>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <mutex>

template <typename... Parameters>
class Delegate
{
public:
    using CallbackFunctionType = std::function<void(Parameters...)>;
    using Ptr = std::shared_ptr<Delegate>;

    Delegate(const CallbackFunctionType& InCallback)
        : Callback(InCallback)
    {
    }

    void Invoke(Parameters... Args)
    {
        Callback(Args...);
    }

private:
    CallbackFunctionType Callback;

};

template <typename DelegateClass>
class Event
{
public:
    using DelegatePtr = typename DelegateClass::Ptr;
    using HookFunction = std::function<void()>;

    DelegatePtr Register(const typename DelegateClass::CallbackFunctionType& Function)
    {
        auto Deleter = [this](DelegateClass* ToDelete) mutable {
            std::scoped_lock lock(Mutex);

            if (auto iter = DelegateSet.find(ToDelete); iter != DelegateSet.end())
            {
                DelegateSet.erase(iter);
            }

            delete ToDelete;

            if (DelegateSet.size() == 0)
            {
                OnLastUnregistered();
            }
        };

        std::scoped_lock lock(Mutex);

        DelegatePtr Result(new DelegateClass(Function), Deleter);

        DelegateSet.insert(Result.get());

        if (DelegateSet.size() == 1)
        {
            OnFirstRegistered();
        }

        return Result;
    }

    template<typename... Parameters>
    void Broadcast(Parameters... Args)
    {
        std::scoped_lock lock(Mutex);

        // We generate a copy of the map before iterating over it to prevent issues in situations
        // where we unregister during the invokation. This needs handling in a better way, this
        // is a waste of performance. 
        std::unordered_set<DelegateClass*> DelegateSetCopy = DelegateSet;
        for (DelegateClass* Delegate : DelegateSetCopy)
        {   
            Delegate->Invoke(Args...);
        }
    }

    void HookFirstRegistered(const HookFunction& Function)
    {
        std::scoped_lock lock(Mutex);

        FirstRegisteredHook = Function;
    }

    void UnhookFirstRegistered()
    {
        std::scoped_lock lock(Mutex);

        FirstRegisteredHook = nullptr;
    }

    void HookLastUnregistered(const HookFunction& Function)
    {
        std::scoped_lock lock(Mutex);

        LastUnregisteredHook = Function;
    }

    void UnhookLastUnregistered()
    {
        std::scoped_lock lock(Mutex);

        LastUnregisteredHook = nullptr;
    }

protected:

    void OnFirstRegistered() 
    {
        std::scoped_lock lock(Mutex);

        if (FirstRegisteredHook)
        {
            FirstRegisteredHook();
        }
    }
    
    void OnLastUnregistered()
    {
        std::scoped_lock lock(Mutex);

        if (LastUnregisteredHook)
        {
            LastUnregisteredHook();
        }
    }

private:
    std::recursive_mutex Mutex;
    std::unordered_set<DelegateClass*> DelegateSet;
    HookFunction FirstRegisteredHook;
    HookFunction LastUnregisteredHook;

};
