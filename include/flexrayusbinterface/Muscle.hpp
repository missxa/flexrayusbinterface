#pragma once

#include <functional>
#include <future>

#include <boost/optional.hpp>
#include "flexrayusbinterface/Mutex.hpp"

enum class Completion
{
  Completed,
  Preempted
};

template <typename T>
class Slot
{
public:
  using result_t = Completion;
  using data_t = boost::optional<std::pair<std::promise<result_t>, T>>;

  template<typename... IntoT>
  auto enqueue(IntoT&&... t_args) -> std::future<result_t>
  {
    auto guard = this->data.get().lock();
    auto& data = guard.get();

    if (data)
      data->first.set_value(Completion::Preempted);

    data = std::pair<std::promise<result_t>, T>{std::promise<result_t>{}, T{std::forward<IntoT>(t_args)...}};
    return data->first.get_future();
  }

  Slot(Mutex<data_t>& data) : data{ data }
  {
  }
  Slot(Slot&&) = default;
  Slot& operator=(Slot&&) = default;

private:
  Slot(Slot const&) = delete;
  Slot& operator=(Slot const&) = delete;

  std::reference_wrapper<Mutex<data_t>> data;
};

template <typename T, typename... Args>
auto make_channel(Args&&... args) -> std::pair<Slot<T>, std::unique_ptr<Mutex<typename Slot<T>::data_t>>>
{
  auto data = new Mutex<typename Slot<T>::data_t>{ std::forward<Args>(args)... };
  return {Slot<T>{*data}, std::unique_ptr<Mutex<typename Slot<T>::data_t>>{data}};
}

/**
 * This class represents the connection between an object `now` that is readily
 * available and another object `later` which will become available in the
 * future in the case when operations on `now` may affect the completion
 * (availability) of `later`.
 *
 * By default (when template parameter `Strict` is `true`), this class only
 * provides access to the two values once `later` becomes available. See method
 * `get` for details.
 *
 * There is a second mode of this class (when template parameter `Strict` is
 * `true` in which the two values can be decoupled and maintaining the
 * relationship between them becomes the user's responsibility. See method
 * `unsafe_get` for details.
 */
template <typename Now, typename Later, typename Strict = std::true_type>
class Entangled
{
public:
  Entangled(Now&& now, std::future<Later>&& later) : now{ std::move(now) }, later{ std::move(later) }
  {
  }

  /**
   * This method decouples the two objects making the user responsible for any
   * misuse caused by operating on `now` while `later` has not yet been
   * computed.
   *
   * This method is only available for the non-strict version of the class
   * (when template parameter `Strict` is `false`) and its name contains the
   * word `unsafe` so that the user should consider the relation between the
   * two objects closely before using it.
   */
  template <typename T = Strict>
  auto unsafe_get() && ->
      typename std::enable_if<std::is_same<T, Strict>::value && !T::value, std::pair<Now, std::future<Later>>>::type
  {
    return { std::move(now), std::move(later) };
  }

  /**
   * Blocks until the `later` object becomes available, when returns both
   * objects.
   */
  auto get() && -> std::pair<Now, Later>
  {
    return { std::move(now), std::move(later).get() };
  }

  auto status() const -> std::future_status
  {
    return later.wait_for(std::chrono::seconds{ 0 });
  }

private:
  Now now;
  std::future<Later> later;
};

/**
 *
 */
template <typename T>
class CompletionGuard
{
public:
  using completion_t = typename Slot<T>::result_t;

  template<typename... IntoT>
  auto enqueue(IntoT&&... t_args) && -> Entangled<typename std::decay<decltype(*this)>::type, completion_t>
  {
    return { CompletionGuard<T>{ std::move(slot) }, slot.enqueue(std::forward<IntoT>(t_args)...) };
  }

  template <typename... Args>
  explicit CompletionGuard(Args&&... args) : slot{ std::forward<Args>(args)... }
  {
  }

  CompletionGuard(CompletionGuard&& other) = default;
  CompletionGuard& operator=(CompletionGuard&& other) = default;

private:
  Slot<T> slot;

  CompletionGuard(CompletionGuard const&) = delete;
  CompletionGuard& operator=(CompletionGuard const&) = delete;
};