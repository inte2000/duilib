#ifndef OBSERVER_IMPL_BASE_HPP
#define OBSERVER_IMPL_BASE_HPP

#include "Utils.h"

namespace DuiLib {

	template <typename ReturnT, typename ParamT>
	class DUILIB_API ReceiverImplBase;

	template <typename ReturnT, typename ParamT>
	class DUILIB_API ObserverImplBase
	{
	public:
		virtual void AddReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
		virtual void RemoveReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
		virtual ReturnT Broadcast(ParamT param) = 0;
		virtual ReturnT RBroadcast(ParamT param) = 0;
		virtual ReturnT Notify(ParamT param) = 0;
	};

	template <typename ReturnT, typename ParamT>
	class DUILIB_API ReceiverImplBase
	{
	public:
		virtual void AddObserver(ObserverImplBase<ReturnT, ParamT>* observer) = 0;
		virtual void RemoveObserver() = 0;
		virtual ReturnT Receive(ParamT param) = 0;
		virtual ReturnT Respond(ParamT param, ObserverImplBase<ReturnT, ParamT>* observer) = 0;
	};

	template <typename ReturnT, typename ParamT>
	class DUILIB_API ReceiverImpl;

	template <typename ReturnT, typename ParamT>
	class DUILIB_API ObserverImpl : public ObserverImplBase<ReturnT, ParamT>
	{
		template <typename ReturnT, typename ParamT>
		friend class Iterator;
	public:
		ObserverImpl()
		{}

		virtual ~ObserverImpl() {}

		virtual void AddReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver)
		{
			if (receiver == NULL)
				return;

			receivers_.Add(receiver);
			receiver->AddObserver(this);
		}

		virtual void RemoveReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver)
		{
			if (receiver == NULL)
				return;

			for (int it = 0; it < receivers_.GetSize(); it++)
			{
				if (receivers_[it] == receiver)
				{
					receivers_.Remove(it);
					break;
				}
			}
		}

		virtual ReturnT Broadcast(ParamT param)
		{
			for (int it = 0; it < receivers_.GetSize(); it++)
			{
				ReceiverImplBase<ReturnT, ParamT>* rv = static_cast<ReceiverImplBase<ReturnT, ParamT>*>(receivers_[it]);
				rv->Receive(param);
			}

			return ReturnT();
		}

		virtual ReturnT RBroadcast(ParamT param)
		{
			if (receivers_.GetSize() > 0)
			{
				for (int it = receivers_.GetSize() - 1; it >= 0; it--)
				{
					ReceiverImplBase<ReturnT, ParamT>* rv = static_cast<ReceiverImplBase<ReturnT, ParamT>*>(receivers_[it]);
					rv->Receive(param);
				}
			}

			return ReturnT();
		}

		virtual ReturnT Notify(ParamT param)
		{
			for (int it = 0; it < receivers_.GetSize(); it++)
			{
				ReceiverImplBase<ReturnT, ParamT>* rv = static_cast<ReceiverImplBase<ReturnT, ParamT>*>(receivers_[it]);
				rv->Respond(param, this);
			}

			return ReturnT();
		}

		template <typename ReturnT, typename ParamT>
		class Iterator
		{
			ObserverImpl<ReturnT, ParamT>& _tbl;
			int index;
			ReceiverImplBase<ReturnT, ParamT>* ptr;
		public:
			Iterator(ObserverImpl& table)
				: _tbl(table), index(0), ptr(NULL)
			{}

			Iterator(const Iterator& v)
				: _tbl(v._tbl), index(v.index), ptr(v.ptr)
			{}

			ReceiverImplBase<ReturnT, ParamT>* next()
			{
				if (index >= _tbl.receivers_.GetSize())
					return NULL;

				for (; index < _tbl.receivers_.GetSize(); )
				{
					ptr = static_cast<ReceiverImplBase<ReturnT, ParamT>*>(_tbl.receivers_[index++]);
					if (ptr)
						return ptr;
				}
				return NULL;
			}
		};

	protected:
		CDuiPtrArray receivers_;
	};


	template <typename ReturnT, typename ParamT>
	class DUILIB_API ReceiverImpl : public ReceiverImplBase<ReturnT, ParamT>
	{
	public:
		ReceiverImpl()
		{}

		virtual ~ReceiverImpl() {}

		virtual void AddObserver(ObserverImplBase<ReturnT, ParamT>* observer)
		{
			observers_.Add(observer);
		}

		virtual void RemoveObserver()
		{
			for (int it = 0; it < observers_.GetSize(); it++)
			{
				ObserverImplBase<ReturnT, ParamT>* oib = static_cast<ObserverImplBase<ReturnT, ParamT>*>(observers_[it]);
				oib->RemoveReceiver(this);
			}
		}

		virtual ReturnT Receive(ParamT param)
		{
			return ReturnT();
		}

		virtual ReturnT Respond(ParamT param, ObserverImplBase<ReturnT, ParamT>* observer)
		{
			return ReturnT();
		}

	protected:
		CDuiPtrArray observers_;
	};

}; //namespace DuiLib


#endif // OBSERVER_IMPL_BASE_HPP