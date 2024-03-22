#pragma once
#include <nlohmann/json.hpp>

namespace scaf {

enum class Performative {
    accept_proposal,
    agree,
    cancel,
    call_for_proposal,
    confirm,
    disconfirm,
    failure,
    inform,
    inform_if,
    inform_ref,
    not_understood,
    propagate,
    propose,
    proxy,
    query_if,
    query_ref,
    refuse,
    reject_proposal,
    request,
    request_when,
    request_whenever,
    subscribe,
};

NLOHMANN_JSON_SERIALIZE_ENUM(Performative, {// clang-format off
    {Performative::accept_proposal, "accept_proposal"},
    {Performative::agree, "agree"},
    {Performative::cancel, "cancel"},
    {Performative::call_for_proposal, "call_for_proposal"},
    {Performative::confirm, "confirm"},
    {Performative::disconfirm, "disconfirm"},
    {Performative::failure, "failure"},
    {Performative::inform, "inform"},
    {Performative::inform_if, "inform_if"},
    {Performative::inform_ref, "inform_ref"},
    {Performative::not_understood, "not_understood"},
    {Performative::propagate, "propagate"},
    {Performative::propose, "propose"},
    {Performative::proxy, "proxy"},
    {Performative::query_if, "query_if"},
    {Performative::query_ref, "query_ref"},
    {Performative::refuse, "refuse"},
    {Performative::reject_proposal, "reject_proposal"},
    {Performative::request, "request"},
    {Performative::request_when, "request_when"},
    {Performative::request_whenever, "request_whenever"},
    {Performative::subscribe, "subscribe"}
})  // clang-format on

}
