#pragma once

namespace NetLib
{
	// LIFO
	template <typename T>
	class CList final
	{
	private:
		template <typename T>
		struct Node
		{
			T info;
			Node* next;
			Node(T val) : info(val), next(nullptr) {}
		};

	public:
		CList() : head(nullptr) {}
		~CList()
		{
			/*
			Node<T>* temp = head;
			while (nullptr != temp)
			{
			head = temp->next;
			delete temp;
			temp = head;
			}
			*/
		}

		void AddFront(const T& value)
		{
			Node<T>* temp = new Node<T>(value);
			temp->next = head;
			head = temp;
		}

		void RemoveFront()
		{
			if (nullptr == head)
				return;
			Node<T>* temp = head;
			head = head->next;
			delete temp;
		}

		T GetFront() const
		{
			if (nullptr != head)
				return head->info;
			else
				return nullptr;
		}

		void MoveNext()
		{
			if (head != nullptr)
			{
				head = head->next;
				return;
			}
		}

		bool IsEmpty() const { return (head == nullptr); }

	private:
		Node<T>* head;
	};
}