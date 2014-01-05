#pragma once





template < typename T >
class XItemList
{
private:
    T*      m_pData;
    DWORD   m_dwLength;

    XItemList(const XItemList&);
    XItemList& operator = (const XItemList&);
public:
    XItemList()
    {
        m_pData = NULL;
        m_dwLength = 0;
    }
    ~XItemList()
    {
        RemoveAll();
    }

    void SetLength(DWORD dwLength)
    {
        RemoveAll();
        m_pData = new T[dwLength];
        m_dwLength = dwLength;
    }
    void RemoveAll()
    {
        if(m_pData)
        {
            delete[] m_pData;
            m_pData = NULL;
            m_dwLength = 0;
        }
    }
    T& GetAt(DWORD dwIndex)
    {
        return m_pData[dwIndex];
    }
    const T& GetAt(DWORD dwIndex) const
    {
        return m_pData[dwIndex];
    }
    DWORD GetLength() const
    {
        return m_dwLength;
    }
};