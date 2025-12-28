// ------------------------------------------------------------
// Copyright 2022 Youyuan Wu
// Licensed under the MIT License (MIT). See License.txt in the repo root for
// license information.
// ------------------------------------------------------------

module;
#include "FabricCommon.h"
#include "FabricRuntime.h"

#ifdef SF_DEBUG
#include <boost/log/trivial.hpp>
#endif

#include <winrt/base.h>

#include <servicefabric/activation_helpers.hpp>
#include <servicefabric/async_context.hpp>
#include <servicefabric/fabric_error.hpp>
#include <servicefabric/string_result.hpp>

#include <memory>

export module kvstore;

export import :curd;
export import :transport;

namespace sf = servicefabric;

class kv_replica
    : public winrt::implements<kv_replica, IFabricStatefulServiceReplica> {
public:
  kv_replica(winrt::com_ptr<IFabricKeyValueStoreReplica2> store,
             std::shared_ptr<kv_server> server)
      : store_(store), server_(std::move(server)),
        role_(FABRIC_REPLICA_ROLE_UNKNOWN) {}

  HRESULT STDMETHODCALLTYPE BeginOpen(
      /* [in] */ FABRIC_REPLICA_OPEN_MODE openMode,
      /* [in] */ IFabricStatefulServicePartition *partition,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginOpen";
#endif
    if (openMode == FABRIC_REPLICA_OPEN_MODE_INVALID) {
      return E_ABORT;
    }

    if (partition == nullptr || callback == nullptr) {
      return E_POINTER;
    }
    HRESULT hr;
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "open mode: " << openMode;
#endif
    FABRIC_SERVICE_PARTITION_INFORMATION info = {};
    const FABRIC_SERVICE_PARTITION_INFORMATION *infoPtr = &info;
    hr = partition->GetPartitionInfo(&infoPtr);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error)
          << "kv_replica::BeginOpen GetPartitionInfo failed: " << hr << " "
          << sf::get_fabric_error_str(hr);
#endif
      return hr;
    }
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "kv_replica::BeginOpen  partition kind: " << info.Kind;
#endif
    hr = store_->BeginOpen(openMode, partition, callback, context);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "kv_replica::BeginOpen failed: " << hr << " "
                               << sf::get_fabric_error_str(hr);
#endif
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE EndOpen(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricReplicator **replicator) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndOpen";
#endif
    HRESULT hr = store_->EndOpen(context, replicator);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "kv_replica::EndOpen failed: " << hr << " "
                               << sf::get_fabric_error_str(hr);
#endif
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE BeginChangeRole(
      /* [in] */ FABRIC_REPLICA_ROLE newRole,
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginChangeRole";
#endif
    HRESULT hr;
    if (newRole == FABRIC_REPLICA_ROLE_PRIMARY) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(debug)
          << "kv_replica::BeginChangeRole primary. Open transport.";
#endif
      hr = server_->open();
      if (hr != S_OK) {
#ifdef SF_DEBUG
        BOOST_LOG_TRIVIAL(error)
            << "kv_replica::BeginChangeRole transport open failed: " << hr;
#endif
        return hr;
      }
    }
    role_ = newRole;
    hr = store_->BeginChangeRole(newRole, callback, context);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginChangeRole failed " << hr;
#endif
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE EndChangeRole(
      /* [in] */ IFabricAsyncOperationContext *context,
      /* [retval][out] */ IFabricStringResult **serviceAddress) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndChangeRole";
#endif
    HRESULT hr;
    if (role_ == FABRIC_REPLICA_ROLE_PRIMARY) {
      std::wstring addr = server_->get_listening_addr();
      // set replica addr to be the transport addr
      winrt::com_ptr<IFabricStringResult> transport_addr =
          winrt::make<sf::string_result>(addr);
      *serviceAddress = transport_addr.detach();
    } else {
      winrt::com_ptr<IFabricStringResult> transport_addr =
          winrt::make<sf::string_result>(L"unknown_addr");
      *serviceAddress = transport_addr.detach();
    }
    winrt::com_ptr<IFabricStringResult> store_address;
    hr = store_->EndChangeRole(context, store_address.put());
    if (hr == S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndChangeRole store_address: "
                               << std::wstring(store_address->get_String());
#endif
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE BeginClose(
      /* [in] */ IFabricAsyncOperationCallback *callback,
      /* [retval][out] */ IFabricAsyncOperationContext **context) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::BeginClose";
#endif
    return store_->BeginClose(callback, context);
  }

  HRESULT STDMETHODCALLTYPE EndClose(
      /* [in] */ IFabricAsyncOperationContext *context) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::EndClose";
#endif
    HRESULT hr;
    hr = server_->close(); // close on unopened is ok
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error)
          << "kv_replica::EndOpen transport close failed: " << hr;
#endif
    }
    return store_->EndClose(context);
  }

  void STDMETHODCALLTYPE Abort(void) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "kv_replica::Abort";
#endif
  }

private:
  winrt::com_ptr<IFabricKeyValueStoreReplica2> store_;
  std::shared_ptr<kv_server> server_;
  FABRIC_REPLICA_ROLE role_;
};

class datastore_event_handler
    : public winrt::implements<datastore_event_handler,
                               IFabricStoreEventHandler> {
public:
  void STDMETHODCALLTYPE OnDataLoss(void) override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "datastore_event_handler::OnDataLoss";
#endif
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
      winrt::com_ptr<IFabricCodePackageActivationContext> activation_ctx)
      : activation_ctx_(activation_ctx) {}
  HRESULT get_port(std::wstring endpoint_name, ULONG &port) override {
    return sf::get_port(activation_ctx_, endpoint_name, port);
  }

private:
  winrt::com_ptr<IFabricCodePackageActivationContext> activation_ctx_;
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
    : public winrt::implements<service_factory, IFabricStatefulServiceFactory> {

public:
  service_factory(std::shared_ptr<resolver> resolver, std::wstring hostname)
      : resolver_(resolver), hostname_(hostname),
        event_handler_(winrt::make<datastore_event_handler>()) {}

  HRESULT STDMETHODCALLTYPE CreateReplica(
      /* [in] */ LPCWSTR serviceTypeName,
      /* [in] */ FABRIC_URI serviceName,
      /* [in] */ ULONG initializationDataLength,
      /* [size_is][in] */ const byte *initializationData,
      /* [in] */ FABRIC_PARTITION_ID partitionId,
      /* [in] */ FABRIC_REPLICA_ID replicaId,
      /* [retval][out] */ IFabricStatefulServiceReplica **serviceReplica)
      override {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug) << "service_factory::CreateReplica";
#endif

    std::string data;
    if (initializationDataLength > 0 && initializationData != nullptr) {
      data = std::string(initializationData,
                         initializationData + initializationDataLength);
    }
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "service_factory::CreateInstance "
        << "serviceTypeName " << serviceTypeName << "serviceName "
        << serviceName << " initializationDataLength "
        << initializationDataLength << " initializationData "
        << data
        // << "partitionId " << partitionId
        << " replicaId " << replicaId;
#else
    UNREFERENCED_PARAMETER(serviceTypeName);
    UNREFERENCED_PARAMETER(serviceName);
    UNREFERENCED_PARAMETER(initializationDataLength);
    UNREFERENCED_PARAMETER(initializationData);
    UNREFERENCED_PARAMETER(partitionId);
#endif
    ULONG port = 0;
    HRESULT hr = resolver_->get_port(L"KvReplicatorEndpoint", port);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "Cannot get port: " << hr;
#endif
      return hr;
    }

    ULONG transport_port = 0;
    hr = resolver_->get_port(L"KvTransportEndpoint", transport_port);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "Cannot get KvTransportEndpoint port: " << hr;
#endif
      return hr;
    }

    std::wstring replicatorAddress = hostname_ + L":" + std::to_wstring(port);
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(debug)
        << "Using replicatorAddress: " << replicatorAddress;
#endif
    FABRIC_REPLICATOR_SETTINGS replicatorSettings = {0};
    replicatorSettings.ReplicatorAddress = replicatorAddress.c_str();
    replicatorSettings.Flags = FABRIC_REPLICATOR_ADDRESS;
    replicatorSettings.Reserved = NULL;

    // open kv store backend
    winrt::com_ptr<IFabricKeyValueStoreReplica2> store;
    hr = FabricCreateKeyValueStoreReplica(
        IID_IFabricKeyValueStoreReplica2, KeyValueStoreName.c_str(),
        partitionId, replicaId, &replicatorSettings,
        FABRIC_LOCAL_STORE_KIND_ESE, NULL, event_handler_.get(),
        (void **)store.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(debug)
          << "service_factory::CreateReplica Failed: " << hr;
#endif
      return hr;
    }

    // create transport server
    std::shared_ptr<kv_server> svr =
        std::make_shared<kv_server>(hostname_, transport_port);

    winrt::com_ptr<IFabricStatefulServiceReplica> replica =
        winrt::make<kv_replica>(store, svr);
    *serviceReplica = replica.detach();
    return S_OK;
  }

private:
  // winrt::com_ptr<IFabricCodePackageActivationContext> activation_ctx_;
  std::shared_ptr<resolver> resolver_;
  std::wstring hostname_;
  winrt::com_ptr<IFabricStoreEventHandler> event_handler_;
};