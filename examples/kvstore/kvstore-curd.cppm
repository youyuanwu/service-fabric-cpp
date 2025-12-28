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

#include <servicefabric/waitable_callback.hpp>

export module kvstore:curd;

namespace sf = servicefabric;

// caller need to rollback if failed
HRESULT commit_helper(winrt::com_ptr<IFabricTransaction> &tx) {
  winrt::com_ptr<sf::IFabricAsyncOperationWaitableCallback> callback =
      winrt::make<sf::FabricAsyncOperationWaitableCallback>();
  winrt::com_ptr<IFabricAsyncOperationContext> ctx;

  HRESULT hr;
  hr = tx->BeginCommit(1000, callback.get(), ctx.put());
  if (hr != S_OK) {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(error) << "cannot BeginCommit: " << hr;
#endif
    return hr;
  }
  callback->Wait();
  FABRIC_SEQUENCE_NUMBER num;
  hr = tx->EndCommit(ctx.get(), &num);
  if (hr != S_OK) {
#ifdef SF_DEBUG
    BOOST_LOG_TRIVIAL(error) << "cannot EndCommit: " << hr;
#endif
    return hr;
  }
  return S_OK;
}

export class curd {
public:
  curd(winrt::com_ptr<IFabricKeyValueStoreReplica2> store) : store_(store) {}

  HRESULT add(std::wstring key, std::string val) {
    HRESULT hr;
    winrt::com_ptr<IFabricTransaction> tx;
    hr = store_->CreateTransaction(tx.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot open tx: " << hr;
#endif
      return hr;
    }

    // check if key exists
    bool b;
    this->contains_key(tx, key, b);
    if (b) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(info) << "key already exist: " << key << hr;
#endif
      tx->Rollback();
      return E_FAIL;
    }
    hr = store_->Add(tx.get(), key.c_str(), static_cast<LONG>(val.size()),
                     (BYTE *)val.c_str());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot do add: " << hr;
#endif
      tx->Rollback();
      return hr;
    }

    hr = commit_helper(tx);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "commit_helper failed: " << hr;
#endif
      tx->Rollback();
      return hr;
    }
    return S_OK;
  }

  HRESULT get(std::wstring key, std::string &val) {
    HRESULT hr;
    winrt::com_ptr<IFabricTransaction> tx;
    hr = store_->CreateTransaction(tx.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot open tx: " << hr;
#endif
      return hr;
    }

    // check if key exists
    bool b;
    hr = this->contains_key(tx, key, b);
    if (!b) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(info) << "key does not exist: " << key << hr;
#endif
      tx->Rollback();
      return E_FAIL;
    }
    FABRIC_SEQUENCE_NUMBER dummy_num;
    hr = this->get_helper(tx, key, val, dummy_num);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "get_helper failed: " << hr;
#endif
      tx->Rollback();
      return hr;
    }

    hr = commit_helper(tx);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "commit_helper failed: " << hr;
#endif
      tx->Rollback();
      return hr;
    }
    return S_OK;
  }

  HRESULT put(std::wstring key, std::string val) {
    HRESULT hr;
    winrt::com_ptr<IFabricTransaction> tx;
    hr = store_->CreateTransaction(tx.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot open tx: " << hr;
#endif
      return hr;
    }

    // check if key exists
    bool b;
    hr = this->contains_key(tx, key, b);
    if (!b) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(info) << "key does not exist: " << key << hr;
#endif
      tx->Rollback();
      return E_FAIL;
    }

    // read the sequence num
    std::string dummy_val;
    FABRIC_SEQUENCE_NUMBER seq_num;
    hr = this->get_helper(tx, key, dummy_val, seq_num);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "get_helper failed: " << hr;
#endif
      tx->Rollback();
      return hr;
    }

    // do put
    hr = this->put_helper(tx, key, val, seq_num);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "put_helper failed: " << hr;
#endif
      tx->Rollback();
      return hr;
    }
    return S_OK;
  }

  HRESULT remove(std::wstring key) {
    HRESULT hr;
    winrt::com_ptr<IFabricTransaction> tx;
    hr = store_->CreateTransaction(tx.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot open tx: " << hr;
#endif
      return hr;
    }

    // check if key exists
    bool b;
    hr = this->contains_key(tx, key, b);
    if (!b) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(info) << "key does not exist: " << key << hr;
#endif
      tx->Rollback();
      return E_FAIL;
    }

    // read the sequence num
    std::string dummy_val;
    FABRIC_SEQUENCE_NUMBER seq_num;
    hr = this->get_helper(tx, key, dummy_val, seq_num);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "get_helper failed: " << hr;
#endif
      tx->Rollback();
      return hr;
    }

    // do remove
    hr = this->remove_helper(tx, key, seq_num);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "put_helper failed: " << hr;
#endif
      tx->Rollback();
      return hr;
    }
    return S_OK;
  }

private:
  // helper functions do not end transaction
  HRESULT contains_key(winrt::com_ptr<IFabricTransaction> &tx,
                       const std::wstring &key, bool &ret) {
    HRESULT hr;
    BOOLEAN b;
    hr = store_->Contains(tx.get(), key.c_str(), &b);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot do contains: " << hr;
#endif
      return hr;
    }
    ret = b;
    return S_OK;
  }

  HRESULT get_helper(winrt::com_ptr<IFabricTransaction> &tx,
                     const std::wstring &key, std::string &val,
                     FABRIC_SEQUENCE_NUMBER &num) {
    HRESULT hr;
    winrt::com_ptr<IFabricKeyValueStoreItemResult> item;
    hr = store_->Get(tx.get(), key.c_str(), item.put());
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot Get: " << hr;
#endif
      return hr;
    }
    const FABRIC_KEY_VALUE_STORE_ITEM *i = item->get_Item();
    const FABRIC_KEY_VALUE_STORE_ITEM_METADATA *meta = i->Metadata;
    int len = meta->ValueSizeInBytes;
    val = std::string(i->Value, i->Value + len);
    num = meta->SequenceNumber;
    return S_OK;
  }

  HRESULT put_helper(winrt::com_ptr<IFabricTransaction> &tx,
                     const std::wstring &key, const std::string &val,
                     FABRIC_SEQUENCE_NUMBER num) {
    HRESULT hr;
    hr = store_->Update(tx.get(), key.c_str(), static_cast<LONG>(val.size()),
                        (BYTE *)val.c_str(), num);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot Update: " << hr;
#endif
      return hr;
    }
    return S_OK;
  }

  HRESULT remove_helper(winrt::com_ptr<IFabricTransaction> &tx,
                        const std::wstring &key, FABRIC_SEQUENCE_NUMBER num) {
    HRESULT hr;
    hr = store_->Remove(tx.get(), key.c_str(), num);
    if (hr != S_OK) {
#ifdef SF_DEBUG
      BOOST_LOG_TRIVIAL(error) << "cannot Remove: " << hr;
#endif
      return hr;
    }
    return S_OK;
  }

  winrt::com_ptr<IFabricKeyValueStoreReplica2> store_;
};