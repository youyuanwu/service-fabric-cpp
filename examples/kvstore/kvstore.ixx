// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

module;
#include "FabricCommon.h"
#include "FabricRuntime.h"
#include <boost/log/trivial.hpp>
#include <moderncom/interfaces.h>

#include <servicefabric/activation_helpers.hpp>
#include <servicefabric/async_context.hpp>
#include <servicefabric/fabric_error.hpp>
#include <servicefabric/string_result.hpp>

#include <memory>

export module kvstore;

import kvtransport;

namespace sf = servicefabric;

class kv_replica
    : public belt::com::object<kv_replica, IFabricStatefulServiceReplica> {
public:
  kv_replica(belt::com::com_ptr<IFabricKeyValueStoreReplica2> store,
             std::shared_ptr<kv_server> server)
      : store_(store), server_(std::move(server)),
        role_(FABRIC_REPLICA_ROLE_UNKNOWN) {}

  HRESULT STDMETHODCALLTYPE BeginOpen(
      /* [in] */ FABRIC_REPLICA_OPEN_MODE openMode,
      /* [in] */ IFabricStatefulServicePartition *partition,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginOpen";

    if (openMode == FABRIC_REPLICA_OPEN_MODE_INVALID) {
      return E_ABORT;
    }

    if (partition == nullptr || callback == nullptr) {
      return E_POINTER;
    }
    HRESULT hr;

    BOOST_LOG_TRIVIAL(debug) << "open mode: " << openMode;
    FABRIC_SERVICE_PARTITION_INFORMATION info = {};
    const FABRIC_SERVICE_PARTITION_INFORMATION *infoPtr = &info;
    hr = partition->GetPartitionInfo(&infoPtr);
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error)
          << "kv_replica::BeginOpen GetPartitionInfo failed: " << hr << " "
          << sf::get_fabric_error_str(hr);
      return hr;
    }
    BOOST_LOG_TRIVIAL(debug)
        << "kv_replica::BeginOpen  partition kind: " << info.Kind;

    hr = store_->BeginOpen(openMode, partition, callback, context);
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "kv_replica::BeginOpen failed: " << hr << " "
                               << sf::get_fabric_error_str(hr);
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE EndOpen(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricReplicator **replicator) override {
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndOpen";
    HRESULT hr = store_->EndOpen(context, replicator);
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "kv_replica::EndOpen failed: " << hr << " "
                               << sf::get_fabric_error_str(hr);
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE BeginChangeRole(
      /* [in] */ FABRIC_REPLICA_ROLE newRole,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginChangeRole";
    HRESULT hr;
    if (newRole == FABRIC_REPLICA_ROLE_PRIMARY) {
      BOOST_LOG_TRIVIAL(debug)
          << "kv_replica::BeginChangeRole primary. Open transport.";
      hr = server_->open();
      if (hr != S_OK) {
        BOOST_LOG_TRIVIAL(error)
            << "kv_replica::BeginChangeRole transport open failed: " << hr;
        return hr;
      }
    }
    role_ = newRole;
    hr = store_->BeginChangeRole(newRole, callback, context);
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginChangeRole failed " << hr;
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE EndChangeRole(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricStringResult **serviceAddress) override {
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndChangeRole";
    HRESULT hr;
    if (role_ == FABRIC_REPLICA_ROLE_PRIMARY) {
      std::wstring addr = server_->get_listening_addr();
      // set replica addr to be the transport addr
      belt::com::com_ptr<IFabricStringResult> transport_addr =
          sf::string_result::create_instance(addr).to_ptr();
      *serviceAddress = transport_addr.detach();
    } else {
      belt::com::com_ptr<IFabricStringResult> transport_addr =
          sf::string_result::create_instance(L"unknown_addr").to_ptr();
      *serviceAddress = transport_addr.detach();
    }
    belt::com::com_ptr<IFabricStringResult> store_address;
    hr = store_->EndChangeRole(context, store_address.put());
    if (hr == S_OK) {
      BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndChangeRole store_address: "
                               << std::wstring(store_address->get_String());
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE BeginClose(
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginClose";
    return store_->BeginClose(callback, context);
  }

  HRESULT STDMETHODCALLTYPE EndClose(
      /* [in] */ IFabricAsyncOperationContext *context) override {
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndClose";
    HRESULT hr;
    hr = server_->close(); // close on unopened is ok
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error)
          << "kv_replica::EndOpen transport close failed: " << hr;
    }
    return store_->EndClose(context);
  }

  void STDMETHODCALLTYPE Abort(void) override {
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::Abort";
  }

private:
  belt::com::com_ptr<IFabricKeyValueStoreReplica2> store_;
  std::shared_ptr<kv_server> server_;
  FABRIC_REPLICA_ROLE role_;
};

class datastore_event_handler
    : public belt::com::object<datastore_event_handler,
                               IFabricStoreEventHandler> {
public:
  void STDMETHODCALLTYPE OnDataLoss(void) override {
    BOOST_LOG_TRIVIAL(debug) << "datastore_event_handler::OnDataLoss";
  }
};

// resolve port for endpoint
export class resolver {
public:
  virtual HRESULT get_port(std::wstring endpoint_name, ULONG &port) = 0;
};

export class sf_resolver : public resolver {
public:
  sf_resolver(
      belt::com::com_ptr<IFabricCodePackageActivationContext> activation_ctx)
      : activation_ctx_(activation_ctx) {}
  HRESULT get_port(std::wstring endpoint_name, ULONG &port) override {
    return sf::get_port(activation_ctx_, endpoint_name, port);
  }

private:
  belt::com::com_ptr<IFabricCodePackageActivationContext> activation_ctx_;
};

export class dummy_resolver : public resolver {
public:
  HRESULT get_port(std::wstring endpoint_name, ULONG &port) override {
    if (endpoint_name == L"KvTransportEndpoint") {
      port = 12345;
      return S_OK;
    }
    if (endpoint_name == L"KvReplicatorEndpoint") {
      port = 12346;
      return S_OK;
    }
    assert(false);
    return E_FAIL;
  }
};

const std::wstring KeyValueStoreName = L"kvstoreKeyValueStore";

export class service_factory
    : public belt::com::object<service_factory, IFabricStatefulServiceFactory> {

public:
  service_factory(std::shared_ptr<resolver> resolver, std::wstring hostname)
      : resolver_(resolver), hostname_(hostname),
        event_handler_(datastore_event_handler::create_instance().to_ptr()) {}

  HRESULT STDMETHODCALLTYPE CreateReplica(
      /* [in] */ LPCWSTR serviceTypeName,
      /* [in] */ FABRIC_URI serviceName,
      /* [in] */ ULONG initializationDataLength,
      /* [size_is][in] */ const byte *initializationData,
      /* [in] */ FABRIC_PARTITION_ID partitionId,
      /* [in] */ FABRIC_REPLICA_ID replicaId,
      /* [retval][out] */ IFabricStatefulServiceReplica **serviceReplica)
      override {
    BOOST_LOG_TRIVIAL(debug) << "service_factory::CreateReplica";

    std::string data;
    if (initializationDataLength > 0 && initializationData != nullptr) {
      data = std::string(initializationData,
                         initializationData + initializationDataLength);
    }

    BOOST_LOG_TRIVIAL(debug)
        << "service_factory::CreateInstance "
        << "serviceTypeName " << serviceTypeName << "serviceName "
        << serviceName << " initializationDataLength "
        << initializationDataLength << " initializationData "
        << data
        // << "partitionId " << partitionId
        << " replicaId " << replicaId;

    ULONG port = 0;
    HRESULT hr = resolver_->get_port(L"KvReplicatorEndpoint", port);
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "Cannot get port: " << hr;
      return hr;
    }

    ULONG transport_port = 0;
    hr = resolver_->get_port(L"KvTransportEndpoint", transport_port);
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(error) << "Cannot get KvTransportEndpoint port: " << hr;
      return hr;
    }

    std::wstring replicatorAddress = hostname_ + L":" + std::to_wstring(port);
    BOOST_LOG_TRIVIAL(debug)
        << "Using replicatorAddress: " << replicatorAddress;

    FABRIC_REPLICATOR_SETTINGS replicatorSettings = {0};
    replicatorSettings.ReplicatorAddress = replicatorAddress.c_str();
    replicatorSettings.Flags = FABRIC_REPLICATOR_ADDRESS;
    replicatorSettings.Reserved = NULL;

    // open kv store backend
    belt::com::com_ptr<IFabricKeyValueStoreReplica2> store;
    hr = FabricCreateKeyValueStoreReplica(
        IID_IFabricKeyValueStoreReplica2, KeyValueStoreName.c_str(),
        partitionId, replicaId, &replicatorSettings,
        FABRIC_LOCAL_STORE_KIND_ESE, NULL, event_handler_.get(),
        (void **)store.put());
    if (hr != S_OK) {
      BOOST_LOG_TRIVIAL(debug)
          << "service_factory::CreateReplica Failed: " << hr;
      return hr;
    }

    // create transport server
    std::shared_ptr<kv_server> svr =
        std::make_shared<kv_server>(hostname_, transport_port);

    belt::com::com_ptr<IFabricStatefulServiceReplica> replica =
        kv_replica::create_instance(store, svr).to_ptr();
    *serviceReplica = replica.detach();
    return S_OK;
  }

private:
  // belt::com::com_ptr<IFabricCodePackageActivationContext> activation_ctx_;
  std::shared_ptr<resolver> resolver_;
  std::wstring hostname_;
  belt::com::com_ptr<IFabricStoreEventHandler> event_handler_;
};