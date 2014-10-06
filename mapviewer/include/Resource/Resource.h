#ifndef RESOURCE_H_GUARD
#define RESOURCE_H_GUARD
#include <string>

class Resource
{
	public:
		const static uint32_t INVALID_ID = 0;
		typedef uint32_t ID;
		enum Type {
			NoneType = 0,
			BitmapType = 1,
			ModelType = 2
		};
		
		Resource(Type type, const std::string &path) : id_(0), refcnt_(0), type_(type), path_(path) { id_ = NextID(); }
		virtual ~Resource() {}
		
		void ref() { refcnt_++; }
		void deref() { refcnt_--; }
		
		bool inUse() { return refcnt_ > 0; }
		
		Type type() { return type_; }
		ID id() { return id_; }
		std::string path() { return path_; }
		
		void pin() { pinned_ = true; }
		void unpin() { pinned_ = false; }
		bool isPinned() { return pinned_; }
		
	private:
		ID id_;
		uint32_t refcnt_;
		Type type_;
		std::string path_;
		bool pinned_;
		
		static ID NextID() { static ID next_id_ = 1; ID tid = next_id_; next_id_++; return tid; }
};

#endif /* RESOURCE_H_GUARD */
