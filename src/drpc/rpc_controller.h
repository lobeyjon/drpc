#pragma once

#include "google/protobuf/service.h"

namespace drpc {

class DRpcController : public google::protobuf::RpcController {
public:
    DRpcController();
    virtual ~DRpcController();

    virtual void Reset();

    virtual bool Failed() const;

    virtual std::string ErrorText() const;

    virtual void StartCancel();

    virtual void SetFailed(const std::string& reason);

    virtual bool IsCanceled() const;

    virtual void NotifyOnCancel(google::protobuf::Closure* callback);
};


}